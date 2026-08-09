#include "pch.h"
#include "CyanoCalculator.h"
#include "CyanoSimulator.h"
#include "DescriptorHeap.h"
#include "RWStructuredBuffer.h"
#include "StructuredBuffer.h"
#include "ReadBackBuffer.h"
#include "Graphic.h"
#include "ComputePipelineStateBuilder.h"
#include "RootSignatureBuilder.h"
#include "Fence.h"
#include "DescriptorHeap.h"

const int CyanoCalculator::CELL_SIZE = 10;
const float CyanoCalculator::PIXEL_AREA_WIDTH = Graphic::ClientWidth * 0.5f;
const float CyanoCalculator::PIXEL_AREA_HEIGHT = Graphic::ClientWidth * 0.5f;

const int CyanoCalculator::GRID_WIDTH =  PIXEL_AREA_WIDTH / CELL_SIZE;
const int CyanoCalculator::GRID_HEIGHT = PIXEL_AREA_HEIGHT / CELL_SIZE;

CyanoCalculator::CyanoCalculator(Graphic& graphic, const UINT maxPointNum)	:
	mShaderVisibleHeap(graphic.getDescriptorHeap())
{
	//パラメータの初期化
	mUploadParams.numPoints = 0;
	mUploadParams.gridWidth = GRID_WIDTH;
	mUploadParams.gridHeight = GRID_HEIGHT;
	mUploadParams.cellSize = CELL_SIZE;

	prepareCommand(*graphic.getDevice());
	prepareFence(*graphic.getDevice());
	prepareDescriptorHeap(*graphic.getDevice(), maxPointNum);
	prepareScatterBuffers(*graphic.getDevice(), maxPointNum);
}

CyanoCalculator::~CyanoCalculator() = default;

void CyanoCalculator::startCalculation(std::vector<XMFLOAT4>& pointsPos)
{
	if (pointsPos.size() == 0) return;
	//点の位置データをGPUバッファにコピー
	mPointsPosBuffer->setData(*mComputeCommandList.Get(), pointsPos.data());

	//GPUで点のセル座標とセルのヒストグラムを計算
	dispatchCellIdxHistogram(pointsPos.size());

	//CPU上のメモリにヒストグラムをコピー
	auto histogram = copyHistogramToCPU();

	//ヒストグラムから排他的累積和を計算
	computeCellStart(histogram);

	//GPUで点をセルインデックスでソートした配列を作成
	dispatchScatter(pointsPos.size());


}

void CyanoCalculator::prepareCommand(ID3D12Device& device)
{
	//コマンドアロケータ作成 (GPU、CPUの非同期処理のためにフレーム数分確保)
	HRESULT hr = device.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mComputeCommandAllocator.GetAddressOf()));
	assert(SUCCEEDED(hr));


	//コマンドリスト作成
	hr = device.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mComputeCommandAllocator.Get(), nullptr, IID_PPV_ARGS(mComputeCommandList.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	//コマンドキュー作成
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//GPUタイムアウトが有効
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //直接コマンドキュー
	hr = device.CreateCommandQueue(&desc, IID_PPV_ARGS(mComputeCommandQueue.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void CyanoCalculator::prepareFence(ID3D12Device& device)
{
	mFence = std::make_unique<Fence>(device);
}

void CyanoCalculator::prepareDescriptorHeap(ID3D12Device& device, const UINT maxPointNum)
{
	//各点のセル番号を書き込むバッファ
	mPointsPosBuffer = std::make_unique<StructuredBuffer>(device, sizeof(RenderData), maxPointNum);
	mCellIdxBuffer = std::make_unique<RWStructuredBuffer>(device, sizeof(UINT), maxPointNum);
	mHistogramBuffer = std::make_unique<RWStructuredBuffer>(device, sizeof(UINT), GRID_WIDTH * GRID_HEIGHT);


	mShaderVisibleDescRange = mShaderVisibleHeap.allocate(NumSlots(3));
	mShaderVisibleHeap.addSRV(*mPointsPosBuffer.get(), mShaderVisibleDescRange->getIndex(0));
	mShaderVisibleHeap.addUAV(*mCellIdxBuffer.get(), mShaderVisibleDescRange->getIndex(1));
	mShaderVisibleHeap.addUAV(*mHistogramBuffer.get(), mShaderVisibleDescRange->getIndex(2));

	//UAVのクリア用ヒープ
	mShaderNoneVisibleHeap = std::make_unique<DescriptorHeap>(device, NumSlots(2), D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	mShaderNoneVisibleDescRange = mShaderNoneVisibleHeap->allocate(NumSlots(2));
	mShaderNoneVisibleHeap->addUAV(*mCellIdxBuffer.get(), mShaderNoneVisibleDescRange->getIndex(0));
	mShaderNoneVisibleHeap->addUAV(*mHistogramBuffer.get(), mShaderNoneVisibleDescRange->getIndex(1));

	//rootSignatureの作成
	auto rootSignatureHistogram = RootSignatureBuilder()
		.addRootConstants(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addSRVTable(0, 1, D3D12_SHADER_VISIBILITY_ALL)
		.addUAVTable(0, 2, D3D12_SHADER_VISIBILITY_ALL)
		.build(device);

	//PSOの作成
	auto histogramPSO = ComputePipelineStateBuilder()
		.setRootSignature(rootSignatureHistogram.Get())
		.setComputeShader("assets/CellIdxHistogramCS.cso")
		.build(device);

	mComputeRootSignatures[ComputeType::HISTOGRAM] = rootSignatureHistogram;
	mComputePipelineStates[ComputeType::HISTOGRAM] = histogramPSO;

}

void CyanoCalculator::prepareScatterBuffers(ID3D12Device& device, const UINT maxPointNum)
{
	//各点のセル番号を書き込むバッファ
	mCellStartBuffer = std::make_unique<StructuredBuffer>(device, sizeof(UINT), GRID_WIDTH * GRID_HEIGHT);
	mCellCursorBuffer = std::make_unique<RWStructuredBuffer>(device, sizeof(UINT), GRID_WIDTH * GRID_HEIGHT);
	mSortedIndexBuffer = std::make_unique<RWStructuredBuffer>(device, sizeof(UINT), maxPointNum);

	mScatterDescRange = mShaderVisibleHeap.allocate(NumSlots(3));
	mShaderVisibleHeap.addUAV(*mCellIdxBuffer.get(), mScatterDescRange->getIndex(0));
	mShaderVisibleHeap.addUAV(*mCellCursorBuffer.get(), mScatterDescRange->getIndex(1));
	mShaderVisibleHeap.addUAV(*mSortedIndexBuffer.get(), mScatterDescRange->getIndex(2));

	//rootSignatureの作成
	auto rootSignatureScatter = RootSignatureBuilder()
		.addRootConstants(0, 4, D3D12_SHADER_VISIBILITY_ALL)
		.addUAVTable(0, 3, D3D12_SHADER_VISIBILITY_ALL)
		.build(device);

	//PSOの作成
	auto scatterPSO = ComputePipelineStateBuilder()
		.setRootSignature(rootSignatureScatter.Get())
		.setComputeShader("assets/ScatterCS.cso")
		.build(device);

	mComputeRootSignatures[ComputeType::SCATTER] = rootSignatureScatter;
	mComputePipelineStates[ComputeType::SCATTER] = scatterPSO;
}

void CyanoCalculator::clearHistogram()
{
    static UINT clearValue[4] = { 0, 0, 0, 0 };
    mComputeCommandList->ClearUnorderedAccessViewUint(
        mShaderVisibleHeap.getGPUHandle(mShaderVisibleDescRange->getIndex(2)),                  // シェーダー可視ヒープ上のGPUハンドル
        mShaderNoneVisibleHeap->getCPUHandle(mShaderNoneVisibleDescRange->getIndex(1)),  // 同じビューを指す、非シェーダー可視ヒープ上のCPUハンドル
        mHistogramBuffer->getBufferOnGPU(),
        clearValue, 0, nullptr
    );
}



void CyanoCalculator::dispatchCellIdxHistogram(const UINT numPoints)
{
	//ルートシグネチャとPSOをセット
	mComputeCommandList->SetComputeRootSignature(mComputeRootSignatures[ComputeType::HISTOGRAM].Get());
	mComputeCommandList->SetPipelineState(mComputePipelineStates[ComputeType::HISTOGRAM].Get());


	//点の数、グリッドのサイズ、セルのサイズをGPUに送る
	mUploadParams.numPoints = numPoints;
	mComputeCommandList->SetComputeRoot32BitConstants(0, 4, &mUploadParams, 0);

	//ディスクリプタテーブルをセット
	mComputeCommandList->SetComputeRootDescriptorTable(1, mShaderVisibleHeap.getGPUHandle(mShaderVisibleDescRange->getIndex(0)));
	mComputeCommandList->SetComputeRootDescriptorTable(2, mShaderVisibleHeap.getGPUHandle(mShaderVisibleDescRange->getIndex(1)));

	//ヒストグラムを0にクリア
	clearHistogram();
	//ディスパッチ
	static const UINT threadGroupSize = 256;
	const UINT numGroups = (numPoints + threadGroupSize - 1) / threadGroupSize;
	mComputeCommandList->Dispatch(numGroups, 1, 1);

	//ヒストグラムへの書き込みが終わるまで、リソースの使用を待機させる
	std::array<D3D12_RESOURCE_BARRIER, 2> barrier = {};
	barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier[0].UAV.pResource = mHistogramBuffer->getBufferOnGPU();
	barrier[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier[1].UAV.pResource = mCellIdxBuffer->getBufferOnGPU();
	mComputeCommandList->ResourceBarrier(2, barrier.data());
	
}

std::vector<UINT> CyanoCalculator::copyHistogramToCPU()
{
    // コマンドリストをクローズ→実行
    mComputeCommandList->Close();
    ID3D12CommandList* lists[] = { mComputeCommandList.Get()};
    mComputeCommandQueue->ExecuteCommandLists(1, lists);

	//GPUの処理を待機する
	mFence->waitGPU(*mComputeCommandQueue.Get());

	//コマンドをリセット
	mComputeCommandAllocator->Reset();
	mComputeCommandList->Reset(mComputeCommandAllocator.Get(), nullptr);

	//CPUメモリへヒストグラムをコピー
	std::vector<UINT> histogram(GRID_WIDTH * GRID_HEIGHT);
	memcpy(histogram.data(), mHistogramBuffer->getBufferOnCPU(), histogram.size() * sizeof(UINT));

	return histogram;
}

void CyanoCalculator::computeCellStart(std::vector<UINT>& histogram)
{
	std::vector<UINT> cellStart(histogram.size());

	//排他的累積和を計算
	UINT sum = 0;
	for (int i = 0; i < histogram.size(); i++) {
		cellStart[i] = sum;
		sum += histogram[i];
	}

	//GPUメモリへコピー
	mCellStartBuffer->setData(*mComputeCommandList.Get(), cellStart.data());
	memcpy(mCellCursorBuffer->getBufferOnCPU(), cellStart.data(), cellStart.size() * sizeof(UINT));

}

void CyanoCalculator::dispatchScatter(const UINT numPoints)
{
	//ルートシグネチャとPSOをセット
	mComputeCommandList->SetComputeRootSignature(mComputeRootSignatures[ComputeType::SCATTER].Get());
	mComputeCommandList->SetPipelineState(mComputePipelineStates[ComputeType::SCATTER].Get());

	//点の数、グリッドのサイズ、セルのサイズをGPUに送る
	mUploadParams.numPoints = numPoints;
	mComputeCommandList->SetComputeRoot32BitConstants(0, 4, &mUploadParams, 0);

	//ディスクリプタテーブルをセット
	mComputeCommandList->SetComputeRootDescriptorTable(1, mShaderVisibleHeap.getGPUHandle(mScatterDescRange->getIndex(0)));

	//ディスパッチ
	static const UINT threadGroupSize = 256;
	const UINT numGroups = (numPoints + threadGroupSize - 1) / threadGroupSize;
	mComputeCommandList->Dispatch(numGroups, 1, 1);

	//ヒストグラムへの書き込みが終わるまで、リソースの使用を待機させる
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = mSortedIndexBuffer->getBufferOnGPU();
	mComputeCommandList->ResourceBarrier(1, &barrier);

	mFence->waitGPU(*mComputeCommandQueue.Get());
	std::vector<UINT> sortedVector(GRID_WIDTH * GRID_HEIGHT);
	memcpy(sortedVector.data(), mSortedIndexBuffer->getBufferOnCPU(), sortedVector.size() * sizeof(UINT));

	
}
