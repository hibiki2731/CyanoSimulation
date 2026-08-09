cbuffer GridParams : register(b0)
{
    uint numPoints;
    uint gridWidth;
    uint gridHeight;
    uint cellSize;
};

RWStructuredBuffer<uint> CellIndexIn : register(u0);
RWStructuredBuffer<uint> CellCursor : register(u1);
RWStructuredBuffer<uint> SortedIndexOut : register(u2);

[numthreads(256, 1, 1)]
void main( uint3 dispatchID : SV_DispatchThreadID )
{
    uint pointIdx = dispatchID.x;
    
    if (pointIdx >= numPoints) return;
    
    uint cellIdx = CellIndexIn[pointIdx];
    
    uint writeSlot;
    InterlockedAdd(CellCursor[cellIdx], 1, writeSlot);
    
    SortedIndexOut[writeSlot] = pointIdx;
}