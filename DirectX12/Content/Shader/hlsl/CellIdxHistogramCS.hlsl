cbuffer GridParams : register(b0)
{
    uint numPoints;
    uint gridWidth;
    uint gridHeight;
    uint cellSize;
}

struct PointData
{
    float4 position;
};

StructuredBuffer<PointData> PointsPos : register(t0);
RWStructuredBuffer<uint> PointsCellIdx : register(u0);
RWStructuredBuffer<uint> CellHistogram : register(u1);

[numthreads(256, 1, 1)]
void main( uint3 dispatchID : SV_DispatchThreadID )
{
    uint pointIdx = dispatchID.x;
    if (pointIdx >= numPoints) return;
    
    float2 pos = PointsPos[pointIdx].position.xy;
    
    //セルの座標を算出
    uint cellX = (uint)(pos.x / (float)cellSize);
    uint cellY = (uint)(pos.y / (float)cellSize);
    
    //範囲外に出ないよう、クランプ
    cellX = min(cellX, gridWidth - 1);
    cellY = min(cellY, gridHeight - 1);
    
    uint cellIdx = cellX + cellY * gridWidth;
    
    PointsCellIdx[pointIdx] = cellIdx;
    
    //ヒストグラムへatomic加算
    uint dummy;
    InterlockedAdd(CellHistogram[cellIdx], 1, dummy);
    
}