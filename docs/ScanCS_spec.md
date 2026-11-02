# ScanCS.hlsl 仕様書

セルヒストグラムの排他的累積和(exclusive prefix sum)をGPUで計算するコンピュートシェーダの仕様。
`CyanoCalculator::computeCellStart` の CPU 実装(`read()` によるGPU待機 + CPUループ + 2回のアップロード)を置き換える。

## 1. 目的と要件

| 項目 | 内容 |
|---|---|
| 入力 | `Histogram[i]` : セル i に存在する点の数(`CellIdxHistogramCS` の出力) |
| 出力 | `CellCursor[i] = Σ Histogram[0..i-1]`(排他的累積和。`CellCursor[0] = 0`) |
| 要素数 N | `gridWidth * gridHeight`(現行 720 × 720 = 518,400) |
| 対応上限 | N ≤ 1,048,576(= 1024 × 1024)。3パス構成の上限 |
| 方式 | Blelloch 法(up-sweep / down-sweep)によるブロック内スキャン + ブロック合計のスキャン + オフセット加算 |
| 制約 | CPU読み戻しとCPU計算を行わない。GPU内で完結する |

出力先は `CellCursor` のみ。`ScatterCS` が `CellCursor` を `InterlockedAdd` で消費するため、`AngleAndMoveCS` が参照する
`CellStart`(SRV)は、スキャン完了後に `CellCursor` から GPU 上でコピーして作る(Scatter より前に行う)。

## 2. 全体構成(3パス)

1ブロック = 1024 要素 = 512 スレッド × 2 要素。ブロック数 `numBlocks = ceil(N / 1024)`(N = 518,400 のとき 507)。

| パス | mode | ディスパッチ | 処理 |
|---|---|---|---|
| 1 | 0 | `numBlocks` グループ | 各ブロックを `Histogram` から読み、ブロック内排他スキャンを `CellCursor` に書く。ブロック合計を `BlockSums[group]` に書く |
| 2 | 1 | 1 グループ | `BlockSums[0..numBlocks-1]` を同じ手順で排他スキャン(インプレース) |
| 3 | 2 | `numBlocks` グループ | `CellCursor[i] += BlockSums[i / 1024]` |

パス間は UAV バリアで依存を保証する(§6)。

```
Histogram ──pass1──▶ CellCursor(ブロック内スキャン) , BlockSums(ブロック合計)
BlockSums ──pass2──▶ BlockSums(ブロックオフセット)
CellCursor + BlockSums ──pass3──▶ CellCursor(全体スキャン)
```

## 3. リソース・バインディング

フォーマットは `ComputeShaderFormat::C4S4RW4`(ルート定数4、SRV 4、UAV 4)。SRV は使わない。

### ルート定数(b0)— 4 × uint32

| 位置 | 名前 | 内容 |
|---|---|---|
| 0 | `mode` | 0 = ブロックスキャン / 1 = ブロック合計のスキャン / 2 = オフセット加算 |
| 1 | `numElements` | そのパスで処理する要素数(mode 0, 2: N、mode 1: numBlocks) |
| 2 | `numBlocks` | ceil(N / 1024) |
| 3 | `pad` | 未使用(0) |

### UAV

| レジスタ | 名前 | 型 | 用途 |
|---|---|---|---|
| u0 | `Histogram` | `RWStructuredBuffer<uint>` | mode 0 の入力(読み取り専用で使う) |
| u1 | `CellCursor` | `RWStructuredBuffer<uint>` | mode 0 の出力、mode 2 の入出力 |
| u2 | `BlockSums` | `RWStructuredBuffer<uint>` | mode 0 の出力、mode 1 の入出力、mode 2 の入力。要素数 ≥ numBlocks |

## 4. スレッド構成

- `[numthreads(512, 1, 1)]`、1グループが1ブロック(1024要素)を担当。
- `groupID.x` = ブロック番号、`groupThreadID.x` = ブロック内スレッド番号 `tid`(0〜511)。
- スレッド `tid` はブロック内の要素 `tid` と `tid + 512` を担当する。グローバル添字は `blockBase + tid`、`blockBase + tid + 512`(`blockBase = groupID.x * 1024`)。
- 範囲外(`>= numElements`)の要素は 0 として読み、書き込まない。
- 共有メモリ `groupshared uint temp[1024 + 32]`。バンクコンフリクト回避のパディングとして添字 `n` を `n + (n >> 5)` に写す。

## 5. アルゴリズム(mode 0 / mode 1 共通のブロックスキャン)

1. **ロード**: `temp[ai + (ai>>5)] = (g0 < n) ? src[g0] : 0`、`temp[bi + (bi>>5)] = (g1 < n) ? src[g1] : 0`
   (src は mode 0 なら `Histogram`、mode 1 なら `BlockSums`)。
2. **up-sweep**: `offset = 1`。`d = 512; d > 0; d >>= 1` の間、バリアののち `tid < d` のスレッドが
   `ai = offset*(2*tid+1)-1`、`bi = offset*(2*tid+2)-1`(各々パディング付き)で `temp[bi] += temp[ai]`。`offset <<= 1`。
3. **根の処理**: `tid == 0` が、最終要素(添字 1023)の値 = ブロック合計を退避し、最終要素を 0 にする。
   mode 0 では合計を `BlockSums[groupID.x]` に書く。
4. **down-sweep**: `d = 1; d < 1024; d <<= 1` の間、`offset >>= 1`、バリアののち `tid < d` のスレッドが
   `t = temp[ai]; temp[ai] = temp[bi]; temp[bi] += t`。
5. **ストア**: バリアののち、範囲内の2要素を `dst` に書く(mode 0 は `CellCursor`、mode 1 は `BlockSums`)。

mode 1 は `numElements = numBlocks ≤ 1024` で 1 グループのみ。ロードを全スレッドが終えてから書き戻すため、インプレースで安全。

### mode 2(オフセット加算)

同じグループ構成で、`add = BlockSums[groupID.x]` を読み、範囲内の2要素に対して `CellCursor[g] += add`。

## 6. 同期と呼び出し側(CyanoCalculator)の責務

```
dispatchCellIdxHistogram     (既存。末尾でHistogramのUAVバリア済み)
computeCellStart:
  params = {0, N, numBlocks}  dispatch(numBlocks)   → バリア(CellCursor, BlockSums)
  params = {1, numBlocks, numBlocks}  dispatch(1)   → バリア(BlockSums)
  params = {2, N, numBlocks}  dispatch(numBlocks)   → バリア(CellCursor)
  mCellStartBuffer.copyFrom(mCellCursorBuffer)      (AngleAndMove用のSRVを更新)
dispatchScatter              (CellCursorを消費)
```

- ルート定数は `dispatch` 時点でコマンドリストにコピーされるため、パスごとにパラメータ構造体を書き換えてよい。
- パス1の入力 `Histogram` は、ヒストグラム生成後の UAV バリアで保護済み。
- `CellCursor` は Scatter により上書き(加算)されるため、各フレームのスキャンで毎回全要素が再計算される。クリア不要。

## 7. 検証方法

- 結果の正しさ: 既存 CPU 実装の出力(`CellStart`)と GPU 実装の `CellCursor` を、デバッグ時に N 要素すべて比較して一致を確認する。
  また `CellCursor[N-1] + Histogram[N-1] == 点数` を確認する。
- 性能: `--bench` で `baseline` と同条件(`--stages 100,1000,5000,10000,20000,30000 --frames 300 --warmup 100`)を測り、
  `cpu_cellstart_ms` と `frame_ms` を比較する。

## 8. 制限・注意

- N > 1,048,576 の場合はパス2が1グループに収まらない(CPU側で assert)。
- 1ブロック 1024 要素は固定。変更する場合は numthreads、共有メモリ、`i / 1024` を同時に直す。
- `BlockSums` は GPU 専用バッファ。CPU からの読み書きは行わない。
