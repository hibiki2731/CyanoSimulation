#pragma once
#include<memory>
#include <d3dx12.h>
#include <array>
#include "Compute/IComputeShader.h"

using Microsoft::WRL::ComPtr;

class ComputeShader : public IComputeShader
{
public :
	~ComputeShader() = default;
	void dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ) override;
	void setStructuredBuffer(class IStructuredBuffer& buffer, UINT position) override;
	void setRWStructuredBuffer(class IRWStructuredBuffer& buffer, UINT position) override;
	void setRootConstants(void* pDataSrc) override;
	void waitWriteBuffer(UINT position) override;
	void clearRWStructuredBuffer(UINT position) override;

private:
	friend class ComputeDevice;
	ComputeShader(ID3D12Device& device, ID3D12RootSignature& rootSignature, class DescriptorHeap& shaderVisibleHeap, class DescriptorHeap& shaderNonVisibleHeap, class CommandManager& commandManager, const std::string& filePath, ComputeShaderFormat format = ComputeShaderFormat::S4_RW4);

	ID3D12RootSignature* mRootSignature;
	ComPtr<ID3D12PipelineState> mPSO;
	class DescriptorHeap* mShaderVisibleHeap;
	class DescriptorHeap* mShaderNonVisibleHeap;
	class CommandManager* mCommandManager;
	std::unique_ptr<class DescriptorSlotRange> mShaderVisibleHeapRange;
	std::unique_ptr<class DescriptorSlotRange> mShaderNonVisibleHeapRange;
	std::vector<class RWStructuredBuffer*> mRegisteredRWBuffers;
	UINT mNumSBuffers;
	UINT mNumRWBuffers;
	void* mUploadPointer;
};

