
cbuffer TestParam : register(b0)
{
    float a;
    float b;
    float c;
    float d;
}

StructuredBuffer<int> InputDataA : register(t0);
RWStructuredBuffer<int> RWDataA : register(u0);
[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint threadID = DTid.x;

    int input = InputDataA[threadID];
    RWDataA[threadID] = input + 1;

}