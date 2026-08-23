/*****************************************************************//**
 * \file   CyanoCalculator.h
 * \brief  
 * 
 * \author h_ka1
 * \date   August 2026
 *********************************************************************/

#pragma once
#include <memory>
#include "Graphic/Core/Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include <array>
#include <d3dx12.h>

class CyanoCalculator
{
public:
	CyanoCalculator(class Graphic& graphic, const UINT maxPointNum);
	~CyanoCalculator();

	void startCalculation(std::vector<XMFLOAT4>& pointsPos);
private:
	void prepareCommand(ID3D12Device& device);
	void prepareFence(ID3D12Device& device);
	void prepareDescriptorHeap(ID3D12Device& device, const UINT maxPointNum);
	void prepareScatterBuffers(ID3D12Device& device, const UINT maxPointNum);

	void clearHistogram();
	void dispatchCellIdxHistogram(const UINT numPoints);
	std::vector<UINT> copyHistogramToCPU();
	void computeCellStart(std::vector<UINT>& histogram);
	void dispatchScatter(const UINT numPoints);


	//空間分割法に用いるパラメータ
	struct GridParams {
		UINT numPoints;
		UINT gridWidth;
		UINT gridHeight;
		UINT cellSize;
	};
	GridParams mUploadParams;
	static const float PIXEL_AREA_WIDTH;
	static const float PIXEL_AREA_HEIGHT;
	static const int GRID_WIDTH;
	static const int GRID_HEIGHT;
	static const int CELL_SIZE;

	//GPUに命令を与えるコマンド
	ComPtr<ID3D12GraphicsCommandList> mComputeCommandList;
	ComPtr<ID3D12CommandQueue> mComputeCommandQueue;
	ComPtr<ID3D12CommandAllocator> mComputeCommandAllocator;
	//CPUとGPUの同期につかうフェンス
	std::unique_ptr<class Fence> mFence;

	//ディスクリプタヒープ
	class DescriptorHeap& mShaderVisibleHeap;
	std::unique_ptr<class DescriptorHeap> mShaderNoneVisibleHeap;
	std::unique_ptr<class DescriptorSlotRange> mShaderVisibleDescRange;
	std::unique_ptr<class DescriptorSlotRange> mShaderNoneVisibleDescRange;

	//点のセルインデックスとヒストグラムを計算するためのバッファ
	std::unique_ptr<class StructuredBuffer> mPointsPosBuffer;
	std::unique_ptr<class RWStructuredBuffer> mCellIdxBuffer;	//各点のセル番号を保存
	std::unique_ptr<class RWStructuredBuffer> mHistogramBuffer;	//各セルに存在する点の数を保存

	//スキャッター用バッファ
	std::unique_ptr<class StructuredBuffer> mCellStartBuffer;
	std::unique_ptr<class RWStructuredBuffer> mCellCursorBuffer;
	std::unique_ptr<class RWStructuredBuffer> mSortedIndexBuffer;
	std::unique_ptr<class DescriptorSlotRange> mScatterDescRange;

	
	//コンピュートシェーダー用ルートシグネチャ、PSO
	enum ComputeType{
		HISTOGRAM,
		SCATTER
	};
	std::array<ComPtr<ID3D12RootSignature>, 3> mComputeRootSignatures;
	std::array<ComPtr<ID3D12PipelineState>, 3> mComputePipelineStates;

	

	//コマンドリスト
	//ディスクリプタヒープ,レンジ
	//ルートシグネチャ、パイプラインステート
};

