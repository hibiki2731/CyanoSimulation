
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

#include <pix3.h>
const int CyanoCalculator::CELL_SIZE = 10;
const float CyanoCalculator::PIXEL_AREA_WIDTH = Graphic::ClientWidth * 0.5f;
const float CyanoCalculator::PIXEL_AREA_HEIGHT = Graphic::ClientWidth * 0.5f;

const int CyanoCalculator::GRID_WIDTH =  static_cast<int>(PIXEL_AREA_WIDTH / CELL_SIZE);
const int CyanoCalculator::GRID_HEIGHT = static_cast<int>(PIXEL_AREA_HEIGHT / CELL_SIZE);

CyanoCalculator::CyanoCalculator(Graphic& graphic, const UINT maxPointNum)	
{
	//パラメータの初期化
	mUploadParams.numPoints = 0;
	mUploadParams.gridWidth = GRID_WIDTH;
	mUploadParams.gridHeight = GRID_HEIGHT;
	mUploadParams.cellSize = CELL_SIZE;

	prepareShaders(maxPointNum);

}

CyanoCalculator::~CyanoCalculator() = default;

void CyanoCalculator::startCalculation(std::vector<XMFLOAT4>& pointsPos)
{
	if (pointsPos.size() == 0) return;
	//点の位置データをGPUバッファにコピー
	mPointsPosBuffer->upload(pointsPos.data(), pointsPos.size() * sizeof(RenderData));

	//GPUで点のセル座標とセルのヒストグラムを計算
	dispatchCellIdxHistogram(static_cast<UINT>(pointsPos.size()));

	//ヒストグラムから排他的累積和を計算
	computeCellStart();

	//GPUで点をセルインデックスでソートした配列を作成
	dispatchScatter(static_cast<UINT>(pointsPos.size()));


}

void CyanoCalculator::prepareShaders(const UINT maxPointNum)
{
	auto& factory = GetEngineResourceFactory();
	auto& computeDevice = GetComputeDevice();
	
	//ヒストグラム作製用シェーダ
	mPointsPosBuffer = factory.createStructuredBuffer(maxPointNum, sizeof(RenderData));
	mCellIdxBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(UINT));
	mHistogramBuffer = factory.createRWStructuredBuffer(GRID_WIDTH * GRID_HEIGHT, sizeof(UINT));

	mHistogramShader = computeDevice.createComputeShader("Content/Shader/cso/CellIdxHistogramCS.cso");
	mHistogramShader->setRootConstants(&mUploadParams);
	mHistogramShader->setStructuredBuffer(*mPointsPosBuffer, 0);
	mHistogramShader->setRWStructuredBuffer(*mCellIdxBuffer, 0);
	mHistogramShader->setRWStructuredBuffer(*mHistogramBuffer, 1);

	//スキャター用シェーダ
	mCellStartBuffer = factory.createStructuredBuffer(GRID_WIDTH * GRID_HEIGHT, sizeof(UINT));
	mCellCursorBuffer = factory.createRWStructuredBuffer(GRID_WIDTH * GRID_HEIGHT, sizeof(UINT));
	mSortedIndexBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(UINT));
	
	mScaterShader = computeDevice.createComputeShader("Content/Shader/cso/ScatterCS.cso");
	mScaterShader->setRootConstants(&mUploadParams);
	mScaterShader->setRWStructuredBuffer(*mCellIdxBuffer, 0);
	mScaterShader->setRWStructuredBuffer(*mCellCursorBuffer, 1);
	mScaterShader->setRWStructuredBuffer(*mSortedIndexBuffer, 2);

	//角度、位置の更新用シェーダ
	mPointsAngleInBuffer = factory.createStructuredBuffer(maxPointNum, sizeof(float));
	mIndividualBeginBuffer = factory.createStructuredBuffer(mNumCyanos, sizeof(UINT));
	mIndividualSizeBuffer = factory.createStructuredBuffer(mNumCyanos, sizeof(UINT));
	mIndividualSpeed = factory.createStructuredBuffer(mNumCyanos, sizeof(float));
	mPointsPosOutBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(RenderData));
	mPointsAngleOutBuffer = factory.createRWStructuredBuffer(maxPointNum, sizeof(float));
	mIndividualheadIdxBuffer = factory.createRWStructuredBuffer(mNumCyanos, sizeof(UINT));
	mIndividualAngularVelocityBuffer = factory.createRWStructuredBuffer(mNumCyanos, sizeof(float));

	mAngleAndMoveShader = computeDevice.createComputeShader("Content/Shader/cso/AngleAndMoveCS.cso");
	mAngleAndMoveShader->setRootConstants(&mSimParams);
	mAngleAndMoveShader->setStructuredBuffer(*mPointsPosBuffer, 0);
	mAngleAndMoveShader->setStructuredBuffer(*mPointsAngleInBuffer, 1);
	mAngleAndMoveShader->setStructuredBuffer(*mCellStartBuffer, 2);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualBeginBuffer, 3);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualSizeBuffer, 4);
	mAngleAndMoveShader->setStructuredBuffer(*mIndividualSpeed, 5);
	mAngleAndMoveShader->setRWStructuredBuffer(*mPointsPosOutBuffer, 0);
	mAngleAndMoveShader->setRWStructuredBuffer(*mHistogramBuffer, 1);
	mAngleAndMoveShader->setRWStructuredBuffer(*mSortedIndexBuffer, 2);
	mAngleAndMoveShader->setRWStructuredBuffer(*mPointsAngleOutBuffer, 3);
	mAngleAndMoveShader->setRWStructuredBuffer(*mIndividualheadIdxBuffer, 4);
	mAngleAndMoveShader->setRWStructuredBuffer(*mIndividualAngularVelocityBuffer, 5);

}

void CyanoCalculator::dispatchCellIdxHistogram(const UINT numPoints)
{
	PIXCaptureParameters captureParams = {};
	captureParams.GpuCaptureParameters.FileName = L"HistogramTest.wpix";
	PIXBeginCapture(PIX_CAPTURE_GPU, &captureParams);

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
	//auto& device = GetComputeDevice();
	//device.executeAndWaitGPU();

	PIXEndCapture(FALSE);

	UINT size = GRID_WIDTH * GRID_HEIGHT;
	std::vector<UINT> histogram(size);
	void* copySrc = mHistogramBuffer->read();
	memcpy(histogram.data(), copySrc, size * sizeof(UINT));

	std::vector<UINT> cellStart(histogram.size());

	//排他的累積和を計算
	UINT sum = 0;
	for (int i = 0; i < histogram.size(); i++) {
		cellStart[i] = sum;
		sum += histogram[i];
	}

	//GPUメモリへコピー
	mCellStartBuffer->upload(cellStart.data(), cellStart.size() * sizeof(UINT));
	mCellCursorBuffer->upload(cellStart.data(), cellStart.size() * sizeof(UINT));
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

	std::vector<UINT> histogram(numPoints);
	memcpy(histogram.data(), mSortedIndexBuffer->read(), numPoints * sizeof(UINT));

	int i = 0;
}
