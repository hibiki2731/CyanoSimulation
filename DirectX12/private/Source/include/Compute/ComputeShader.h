#pragma once
#include<memory>
#include <d3dx12.h>
#include "Compute/IComputeShader.h"
using Microsoft::WRL::ComPtr;

class ComputeShader : public IComputeShader
{
public :
	void dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) override;
	void setStructuredBuffer(class IStructuredBuffer& buffer, UINT position) override;
	void setRWStructuredBuffer(class IRWStructuredBuffer& buffer, UINT position) override;
	void setRootConstants(void* dataSrc) override;
	void waitWriteBuffer(class IRWStructuredBuffer& buffer);
	void clearRWStructuredBuffer(UINT position) override;

private:
	friend class EngineResourceFactory;
	ComputeShader(ID3D12Device& device, ID3D12RootSignature& rootSignature, class DescriptorHeap& shaderVisibleHeap, class DescriptorHeap& shaderNonVisibleHeap, class Command& command, const std::string& filePath);

	ID3D12RootSignature* mRootSignature;
	ComPtr<ID3D12PipelineState> mPSO;
	class DescriptorHeap* mShaderVisibleHeap;
	class DescriptorHeap* mShaderNonVisibleHeap;
	class Command* mCommand;
	std::unique_ptr<class DescriptorSlotRange> mShaderVisibleHeapRange;
	std::unique_ptr<class DescriptorSlotRange> mShaderNonVisibleHeapRange;

};

