
#include "CyanoCalculator.h"
#include "CyanoSimulator.h"
#include "Memory/DescriptorHeap.h"
#include "Memory/RWStructuredBuffer.h"
#include "Memory/StructuredBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Builder/ComputePipelineStateBuilder.h"
#include "Builder/RootSignatureBuilder.h"
#include "Graphic/Core/Fence.h"
#include "Compute/ComputeDevice.h"
#include "Compute/ComputeShader.h"
#include "Builder/EngineResourceFactory.h"
#include <random>

#include <pix3.h>
CyanoCalculator::CyanoCalculator(Graphic& graphic, const UINT maxPointNum, const UINT maxNumIndividuals, std::unique_ptr<IRWStructuredBuffer>& pointsPosOut, std::unique_ptr<IRWStructuredBuffer>& interactionBuffer, ComputeParam& param)	
	:mComputeParam(param),
	mPointsPosOutBuffer(pointsPosOut),
	mPointsIsInteractionBuffer(interactionBuffer),
	mMaxNumCyanos(maxNumIndividuals)
{
	//パラメータの初期化
	mUploadParams.numPoints = 0;
	mUploadParams.gridWidth = param.gridWidth;
	mUploadParams.gridHeight = param.gridHeight;
	mUploadParams.cellSize = param.cellSize;

	prepareShaders(maxPointNum);

}

CyanoCalculator::~CyanoCalculator() = default;

void CyanoCalculator::startCalculation(UINT numPoints, UINT numIndividuals)
{
	if (numPoints == 0) return;

	//点の位置データをGPUバッファにコピー(使用中の点の範囲のみ)
	mPointsPosInBuffer->copyRegionFrom(*mPointsPosOutBuffer, 0, 0, numPoints * sizeof(RenderData));

	//GPUで点のセル座標とセルのヒストグラムを計算
	dispatchCellIdxHistogram(numPoints);

	//ヒストグラムから排他的累積和を計算
	computeCellStart();

	//GPUで点をセルインデックスでソートした配列を作成
	dispatchScatter(numPoints);

	dispatchAngleAndMove(numPoints, numIndividuals);

}

void CyanoCalculator::uploadIndividualBegin(std::vector<UINT>& srcData, const UINT startIndividualIndex)
{
	const UINT byteOffset = startIndividualIndex * sizeof(UINT);
	mIndividualBeginBuffer->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startIndividualIndex) * sizeof(UINT));
}

void CyanoCalculator::uploadIndividualSize(std::vector<UINT>& srcData, const UINT startIndividualIndex)
{
	const UINT byteOffset = startIndividualIndex * sizeof(UINT);
	mIndividualSizeBuffer->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startIndividualIndex) * sizeof(UINT));
}

void CyanoCalculator::uploadIndividualSpeed(std::vector<float>& srcData, const UINT startIndividualIndex)
{
	const UINT byteOffset = startIndividualIndex * sizeof(float);
	mIndividualSpeed->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startIndividualIndex) * sizeof(float));
}

void CyanoCalculator::uploadIndividualHeadIdx(std::vector<UINT>& srcData, const UINT startIndividualIndex)
{
	const UINT byteOffset = startIndividualIndex * sizeof(UINT);
	mIndividualheadIdxBuffer->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startIndividualIndex) * sizeof(UINT));
}

void CyanoCalculator::uploadIndividualAngularVelocity(std::vector<float>& srcData, const UINT startIndividualIndex)
{
	const UINT byteOffset = startIndividualIndex * sizeof(float);
	mIndividualAngularVelocityBuffer->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startIndividualIndex) * sizeof(float));

}

void CyanoCalculator::uploadAngle(std::vector<float>& srcData, const UINT startPointsIndex)
{
	const UINT byteOffset = startPointsIndex * sizeof(float);
	mPointsAngleOutBuffer->uploadToGPU(srcData.data(), byteOffset, byteOffset, (srcData.size() - startPointsIndex) * sizeof(float));
}

void CyanoCalculator::prepareShaders(const UINT maxPointNum)
{
	auto& factory = GetEngineResourceFactory();
	auto& computeDevice = GetComputeDevice();
	
	UINT gridWidth = mComputeParam.gridWidth;
	UINT gridHeight = mComputeParam.gridHeight;

	//ヒストグラム作製用シェーダ
	mPointsPosInBuffer = factory.createStructuredBuffer(maxPointNum, sizeof(RenderData));
	mCellIdxBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(UINT));
	mHistogramBuffer = factory.createRWStructuredBuffer(gridWidth * gridHeight, sizeof(UINT));

	mHistogramShader = computeDevice.createComputeShader("Content/Shader/cso/CellIdxHistogramCS.cso");
	mHistogramShader->setRootConstants(&mUploadParams);
	mHistogramShader->setStructuredBuffer(*mPointsPosInBuffer, 0);
	mHistogramShader->setRWStructuredBuffer(*mCellIdxBuffer, 0);
	mHistogramShader->setRWStructuredBuffer(*mHistogramBuffer, 1);

	//スキャター用シェーダ
	mCellStartBuffer = factory.createStructuredBuffer(gridWidth * gridHeight, sizeof(UINT));
	mCellCursorBuffer = factory.createRWStructuredBuffer(gridWidth * gridHeight, sizeof(UINT));
	mSortedIndexBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(UINT));
	
	//スキャン用シェーダ。ブロック合計のスキャンは1グループで行うため、numBlocksは1024以下
	const UINT numBlocks = (gridWidth * gridHeight + ScanBlockSize - 1) / ScanBlockSize;
	assert(numBlocks <= ScanBlockSize);
	mBlockSumsBuffer = factory.createRWStructuredBuffer(numBlocks, sizeof(UINT));
	mScanParams = { 0, gridWidth * gridHeight, numBlocks, 0 };

	mScanShader = computeDevice.createComputeShader("Content/Shader/cso/ScanCS.cso");
	mScanShader->setRootConstants(&mScanParams);
	mScanShader->setRWStructuredBuffer(*mHistogramBuffer, 0);
	mScanShader->setRWStructuredBuffer(*mCellCursorBuffer, 1);
	mScanShader->setRWStructuredBuffer(*mBlockSumsBuffer, 2);

	mScaterShader = computeDevice.createComputeShader("Content/Shader/cso/ScatterCS.cso");
	mScaterShader->setRootConstants(&mUploadParams);
	mScaterShader->setRWStructuredBuffer(*mCellIdxBuffer, 0);
	mScaterShader->setRWStructuredBuffer(*mCellCursorBuffer, 1);
	mScaterShader->setRWStructuredBuffer(*mSortedIndexBuffer, 2);

	//角度、位置の更新用シェーダ
	mPointsAngleInBuffer = factory.createStructuredBuffer(maxPointNum, sizeof(float));
	mIndividualBeginBuffer = factory.createStructuredBuffer(mMaxNumCyanos, sizeof(UINT));
	mIndividualSizeBuffer = factory.createStructuredBuffer(mMaxNumCyanos, sizeof(UINT));
	mIndividualSpeed = factory.createStructuredBuffer(mMaxNumCyanos, sizeof(float));
	mPointsAngleOutBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(float));
	mIndividualheadIdxBuffer = factory.createRWStructuredBuffer(mMaxNumCyanos, sizeof(UINT));
	mIndividualAngularVelocityBuffer = factory.createRWStructuredBuffer(mMaxNumCyanos, sizeof(float));

	mAngleAndMoveShader = computeDevice.createComputeShader("Content/Shader/cso/AngleAndMoveCS.cso", ComputeShaderFormat::C14S8RW8);
	mAngleAndMoveShader->setRootConstants(&mComputeParam);
	mAngleAndMoveShader->setStructuredBuffer(*mPointsPosInBuffer, 0);
	mAngleAndMoveShader->setStructuredBuffer(*mPointsAngleInBuffer, 1);
	mAngleAndMoveShader->setStructuredBuffer(*mCellStartBuffer, 2);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualBeginBuffer, 3);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualSizeBuffer, 4);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualSpeed, 5);
	mAngleAndMoveShader->setRWStructuredBuffer(*mPointsPosOutBuffer, 0);
	mAngleAndMoveShader->setRWStructuredBuffer(*mPointsAngleOutBuffer, 1);
	mAngleAndMoveShader->setRWStructuredBuffer(*mIndividualheadIdxBuffer, 2);
	mAngleAndMoveShader->setRWStructuredBuffer(*mIndividualAngularVelocityBuffer, 3);
	mAngleAndMoveShader->setRWStructuredBuffer(*mHistogramBuffer, 4);
	mAngleAndMoveShader->setRWStructuredBuffer(*mSortedIndexBuffer, 5);
	mAngleAndMoveShader->setRWStructuredBuffer(*mPointsIsInteractionBuffer, 6);

}

void CyanoCalculator::dispatchCellIdxHistogram(const UINT numPoints)
{
	//点の数、グリッドのサイズ、セルのサイズをGPUに送る
	mUploadParams.numPoints = numPoints;
	//ヒストグラムを0にクリア
	mHistogramShader->clearRWStructuredBuffer(1);
	//ディスパッチ
	static const UINT threadGroupSize = 256;
	const UINT numGroups = (numPoints + threadGroupSize - 1) / threadGroupSize;
	mHistogramShader->dispatch(numGroups, 1, 1);

	mHistogramShader->waitWriteBuffer(0);
	mHistogramShader->waitWriteBuffer(1);
}

void CyanoCalculator::computeCellStart()
{
	const UINT numElements = mScanParams.numElements;
	const UINT numBlocks = mScanParams.numBlocks;

	//パス1 : 各ブロックをスキャンし、ブロック合計を書き出す
	mScanParams.mode = 0;
	mScanShader->dispatch(numBlocks, 1, 1);
	mScanShader->waitWriteBuffer(1);
	mScanShader->waitWriteBuffer(2);

	//パス2 : ブロック合計をスキャンして、各ブロックのオフセットにする
	mScanParams.mode = 1;
	mScanParams.numElements = numBlocks;
	mScanShader->dispatch(1, 1, 1);
	mScanShader->waitWriteBuffer(2);

	//パス3 : ブロックのオフセットを加算
	mScanParams.mode = 2;
	mScanParams.numElements = numElements;
	mScanShader->dispatch(numBlocks, 1, 1);
	mScanShader->waitWriteBuffer(1);

	//AngleAndMoveが参照するセルの先頭位置は、Scatterが加算する前のスキャン結果をコピーして作る
	mCellStartBuffer->copyFrom(*mCellCursorBuffer);
}

void CyanoCalculator::dispatchScatter(const UINT numPoints)
{
	//点の数、グリッドのサイズ、セルのサイズをGPUに送る
	mUploadParams.numPoints = numPoints;

	//ディスパッチ
	static const UINT threadGroupSize = 256;
	const UINT numGroups = (numPoints + threadGroupSize - 1) / threadGroupSize;
	mScaterShader->dispatch(numGroups, 1, 1);

	//ヒストグラムへの書き込みが終わるまで、リソースの使用を待機させる
	mScaterShader->waitWriteBuffer(1);

}

void CyanoCalculator::dispatchAngleAndMove(const UINT numPoints, const UINT numIndividuals)
{
	//参照用の角度を更新(使用中の点の範囲のみ)
	mPointsAngleInBuffer->copyRegionFrom(*mPointsAngleOutBuffer, 0, 0, numPoints * sizeof(float));

	mComputeParam.numIndividuals = numIndividuals;
	static std::random_device seed_gen;
	unsigned seed = seed_gen();
	mComputeParam.frameSeed = seed;

	static const UINT threadGroupSize = 256;
	const UINT numGroups = (numIndividuals + threadGroupSize - 1) / threadGroupSize;
	mAngleAndMoveShader->dispatch(numGroups, 1, 1);
}
