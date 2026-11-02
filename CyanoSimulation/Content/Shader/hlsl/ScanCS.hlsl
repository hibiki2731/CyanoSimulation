// セルヒストグラムの排他的累積和(Blelloch法)。仕様は docs/ScanCS_spec.md
//  mode 0 : Histogram をブロック(1024要素)ごとにスキャンし CellCursor へ。ブロック合計を BlockSums へ
//  mode 1 : BlockSums をスキャン(1グループ、インプレース)
//  mode 2 : CellCursor[i] += BlockSums[i / 1024]

cbuffer ScanParams : register(b0)
{
    uint mode;
    uint numElements;
    uint numBlocks;
    uint pad;
};

RWStructuredBuffer<uint> Histogram : register(u0);
RWStructuredBuffer<uint> CellCursor : register(u1);
RWStructuredBuffer<uint> BlockSums : register(u2);

#define THREADS 512
#define BLOCK_SIZE (THREADS * 2)
#define LOG_NUM_BANKS 5

// バンクコンフリクト回避のパディング
#define CFO(n) ((n) >> LOG_NUM_BANKS)

groupshared uint temp[BLOCK_SIZE + (BLOCK_SIZE >> LOG_NUM_BANKS)];

[numthreads(THREADS, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    const uint tid = groupThreadID.x;
    const uint blockBase = groupID.x * BLOCK_SIZE;
    const uint g0 = blockBase + tid;
    const uint g1 = blockBase + tid + THREADS;

    // mode 2 : ブロックオフセットの加算
    if (mode == 2)
    {
        const uint add = BlockSums[groupID.x];
        if (g0 < numElements)
            CellCursor[g0] += add;
        if (g1 < numElements)
            CellCursor[g1] += add;
        return;
    }

    // ロード(範囲外は0)
    const uint ai0 = tid;
    const uint bi0 = tid + THREADS;
    uint v0 = 0;
    uint v1 = 0;
    if (mode == 0)
    {
        if (g0 < numElements)
            v0 = Histogram[g0];
        if (g1 < numElements)
            v1 = Histogram[g1];
    }
    else
    {
        if (g0 < numElements)
            v0 = BlockSums[g0];
        if (g1 < numElements)
            v1 = BlockSums[g1];
    }
    temp[ai0 + CFO(ai0)] = v0;
    temp[bi0 + CFO(bi0)] = v1;

    // up-sweep
    uint offset = 1;
    for (uint d = THREADS; d > 0; d >>= 1)
    {
        GroupMemoryBarrierWithGroupSync();
        if (tid < d)
        {
            uint ai = offset * (2 * tid + 1) - 1;
            uint bi = offset * (2 * tid + 2) - 1;
            ai += CFO(ai);
            bi += CFO(bi);
            temp[bi] += temp[ai];
        }
        offset <<= 1;
    }

    // 根の処理 : ブロック合計を取り出して0にする
    if (tid == 0)
    {
        const uint last = (BLOCK_SIZE - 1) + CFO(BLOCK_SIZE - 1);
        if (mode == 0)
            BlockSums[groupID.x] = temp[last];
        temp[last] = 0;
    }

    // down-sweep
    for (uint d2 = 1; d2 < BLOCK_SIZE; d2 <<= 1)
    {
        offset >>= 1;
        GroupMemoryBarrierWithGroupSync();
        if (tid < d2)
        {
            uint ai = offset * (2 * tid + 1) - 1;
            uint bi = offset * (2 * tid + 2) - 1;
            ai += CFO(ai);
            bi += CFO(bi);
            const uint t = temp[ai];
            temp[ai] = temp[bi];
            temp[bi] += t;
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // ストア
    const uint r0 = temp[ai0 + CFO(ai0)];
    const uint r1 = temp[bi0 + CFO(bi0)];
    if (mode == 0)
    {
        if (g0 < numElements)
            CellCursor[g0] = r0;
        if (g1 < numElements)
            CellCursor[g1] = r1;
    }
    else
    {
        // mode 1 : インプレース(全スレッドがロードを終えてから書き戻している)
        if (g0 < numElements)
            BlockSums[g0] = r0;
        if (g1 < numElements)
            BlockSums[g1] = r1;
    }
}
