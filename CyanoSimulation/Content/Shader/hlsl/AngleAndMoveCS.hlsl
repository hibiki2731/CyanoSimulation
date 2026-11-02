
cbuffer Param : register(b0)
{
    uint numIndividuals;
    float interactionIntensity;
    float interactionRange;
    float pecletNumber;
    float deltaT;
    float deltaTsqrt;
    float pixelParamRatio;
    float areaWidth;
    float areaHeight;
    uint gridWidth;
    uint gridHeight;
    uint cellSize;
    uint frameSeed;
};

struct PointData
{
    float4 position;
};

StructuredBuffer<PointData> PointsPosIn     : register(t0);
StructuredBuffer<float>     PointsAngleIn : register(t1);
StructuredBuffer<uint>      CellStart       : register(t2);
StructuredBuffer<uint>      IndividualBegin : register(t3);
StructuredBuffer<uint>      IndividualSize  : register(t4);
StructuredBuffer<float>     IndividualSpeed : register(t5);

RWStructuredBuffer<PointData>   PointsPosOut                : register(u0);
RWStructuredBuffer<float>       PointsAngleOut              : register(u1);
RWStructuredBuffer<uint>        IndividualHeadIdx           : register(u2);
RWStructuredBuffer<float>       IndividualAngularVelocity   : register(u3);
RWStructuredBuffer<uint>        Histogram                   : register(u4);
RWStructuredBuffer<uint>        SortedIndex                 : register(u5);
RWStructuredBuffer<float>       IndividualInteraction       : register(u6);

uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb532du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

float randUniform(uint seed)
{
    return (float) hash(seed) / 4294967295.0;
}

float randNormal(uint seed)
{
    float u1 = max(randUniform(seed), 1e-6f);
    float u2 = randUniform(seed ^ 0x9e3779b9u);
    return sqrt(-2.0f * log(u1)) * cos(6.2831853f * u2);
}

uint wrapCellIdx(uint cellX, uint cellY)
{
    uint wrappedCellX = (cellX % gridWidth + gridWidth) % gridWidth;
    uint wrappedCellY = (cellY % gridHeight + gridHeight) % gridHeight;
    return wrappedCellX + wrappedCellY * gridWidth;
}

void calcInteract(inout float interactSum, inout uint interactNum, float2 basePos, float baseAngle, uint cellIdx, uint selfBegin, uint selfSize)
{
    uint start = CellStart[cellIdx];
    uint count = Histogram[cellIdx];
    
    for (uint i = 0; i < count; i++)
    {
        uint otherIdx = SortedIndex[start + i];
        //自分自身だったらスキップ
        if (otherIdx >= selfBegin && otherIdx < selfBegin + selfSize)
            continue;
        
        float2 otherPos = PointsPosIn[otherIdx].position.xy;
        if (distance(basePos, otherPos) < interactionRange * pixelParamRatio)
        {
            float otherAngle = PointsAngleIn[otherIdx];
            interactSum += sin(2 * (baseAngle - otherAngle));
            interactNum++;

        }
    }

}

static const float ROOT2 = 1.41421356f;

[numthreads(256, 1, 1)]
void main( uint3 dispatchId : SV_DispatchThreadID )
{
    //IDの取得
    uint indivIdx = dispatchId.x;
    //IDが菌体数より多ければ処理をスキップ
    if (indivIdx >= numIndividuals)
        return;

    const uint headIdx = IndividualHeadIdx[indivIdx];
    const uint selfBegin = IndividualBegin[indivIdx];
    const uint selfSize = IndividualSize[indivIdx];
    const float speed = IndividualSpeed[indivIdx];
    
    const float headAngle = PointsAngleIn[headIdx];
    const float2 headPos = PointsPosIn[headIdx].position.xy;
    
    //角度の更新
    const uint cellX = (uint) (headPos.x / (float) cellSize);
    const uint cellY = (uint) (headPos.y / (float) cellSize);
    
    //相互作用の計算
    float interactSum = 0.0f;
    uint interactNum = 0;
    
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            const uint neighborCell = wrapCellIdx(cellX + dx, cellY + dy);
            
            calcInteract(interactSum, interactNum, headPos, headAngle, neighborCell, selfBegin, selfSize);
        }
    }
    
    uint newHeadIdx = (headIdx + 1 >= selfBegin + selfSize) ? selfBegin : headIdx + 1;
    
    float interactionValue = interactNum == 0 ? 0.0f : interactSum / (float) interactNum;
    IndividualInteraction[newHeadIdx] = 0.0f;
    
    const float noise = ROOT2 / pecletNumber * (-0.0f + randNormal(frameSeed + indivIdx));

    const float preOmega = IndividualAngularVelocity[indivIdx];
    
    const float d_theta = preOmega - interactionIntensity * interactionValue;
    const float deltaTheta = d_theta * deltaT;
    const float deterministic_term = -deltaTheta;
    const float stochastic_term = noise * deltaTsqrt;
    const float deltaOmega = deterministic_term + stochastic_term;
    
    const float newHeadAngle = headAngle + deltaTheta;
    
    //移動
    const float2 dir = float2(cos(newHeadAngle), sin(newHeadAngle));
    float2 newHeadPos = headPos + dir * (speed * deltaT * pixelParamRatio);
    
    //PBCラップ
    newHeadPos = fmod(newHeadPos + float2(areaWidth, areaHeight), float2(areaWidth, areaHeight));
    
    
    //書き込み
    PointsAngleOut[newHeadIdx] = newHeadAngle;
    
    PointData outPos;
    outPos.position = float4(newHeadPos, 0.0f, 1.0f);
    PointsPosOut[newHeadIdx] = outPos;
    
    IndividualAngularVelocity[indivIdx] = preOmega + deltaOmega;
    IndividualHeadIdx[indivIdx] = newHeadIdx;
    
}