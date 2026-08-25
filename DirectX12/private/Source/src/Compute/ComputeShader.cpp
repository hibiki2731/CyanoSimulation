#include "pch.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"
#include "Command/Command.h"
#include "Builder/ComputePipelineStateBuilder.h"

void ComputeShader::dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
}

void ComputeShader::setStructuredBuffer(IStructuredBuffer& buffer, UINT position)
{
}

void ComputeShader::setRWStructuredBuffer(IRWStructuredBuffer& buffer, UINT position)
{
}

void ComputeShader::setRootConstants(void* dataSrc)
{
}

void ComputeShader::clearRWStructuredBuffer(UINT position)
{
}

ComputeShader::ComputeShader(ID3D12Device& device, ID3D12RootSignature& rootSignature, DescriptorHeap& shaderVisibleHeap, DescriptorHeap& shaderNonVisibleHeap, Command& command, const std::string& shaderFilePath)
	:
	mRootSignature(rootSignature),
	mShaderVisibleHeap(shaderVisibleHeap),
	mShaderNonVisibleHeap(shaderNonVisibleHeap),
	mCommand(command)
{
	//パイプラインステートの初期化
	mPSO = ComputePipelineStateBuilder()
		.setRootSignature(&mRootSignature)
		.setComputeShader(shaderFilePath)
		.build(device);

	//ビューを追加するディスクリプタヒープの領域を確保
	mShaderVisibleHeapRange = mShaderVisibleHeap.allocate(NumSlots(4));
	mShaderNonVisibleHeapRange = mShaderNonVisibleHeap.allocate(NumSlots(4));

}
