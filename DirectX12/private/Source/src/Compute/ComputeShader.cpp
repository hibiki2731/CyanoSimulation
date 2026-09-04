#include "pch.h"
#include "Compute/ComputeShader.h"
#include "Memory/DescriptorHeap.h"
#include "Command/Command.h"
#include "Builder/ComputePipelineStateBuilder.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/RWStructuredBuffer.h"

void ComputeShader::dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
	auto& list = mCommandManager->getComputeCommandList();

	//ディスクリプタヒープをセット
	list->SetDescriptorHeaps(1, mShaderVisibleHeap->getAddress());

	//ルートシグネチャとPSOをセット
	list->SetComputeRootSignature(mRootSignature);
	list->SetPipelineState(mPSO.Get());

	//ルートコンスタントを設定
	list->SetComputeRoot32BitConstants(0, 4, &mUploadParams, 0);

	//ディスクリプタテーブルの設定
	list->SetComputeRootDescriptorTable(1, mShaderVisibleHeap->getGPUHandle(mShaderVisibleHeapRange->getIndex(0)));
	list->SetComputeRootDescriptorTable(2, mShaderVisibleHeap->getGPUHandle(mShaderVisibleHeapRange->getIndex(4)));


	list->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

}

void ComputeShader::setStructuredBuffer(IStructuredBuffer& buffer, UINT position)
{
	assert(position < 4);
	mShaderVisibleHeap->addSRV(*static_cast<StructuredBuffer*>(&buffer), mShaderVisibleHeapRange->getIndex(position));
}

void ComputeShader::setRWStructuredBuffer(IRWStructuredBuffer& buffer, UINT position)
{
	assert(position < 4);
	const int startPos = 4;
	mShaderVisibleHeap->addUAV(*static_cast<RWStructuredBuffer*>(&buffer), mShaderVisibleHeapRange->getIndex(startPos + position));
	mShaderNonVisibleHeap->addUAV(*static_cast<RWStructuredBuffer*>(&buffer), mShaderNonVisibleHeapRange->getIndex(position));
}

void ComputeShader::setRootConstants(void* dataSrc)
{
	mUploadParams = dataSrc;
}

void ComputeShader::waitWriteBuffer(IRWStructuredBuffer& buffer)
{
	//書き込みが終わるまで、リソースの使用を待機させる
	std::array<D3D12_RESOURCE_BARRIER, 1> barrier = {};
	barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier[0].UAV.pResource = static_cast<RWStructuredBuffer*>(&buffer)->getGPUResource();
	mCommandManager->getComputeCommandList()->ResourceBarrier(1, barrier.data());
}

void ComputeShader::clearRWStructuredBuffer(UINT position)
{
}

ComputeShader::ComputeShader(ID3D12Device& device, ID3D12RootSignature& rootSignature, DescriptorHeap& shaderVisibleHeap, DescriptorHeap& shaderNonVisibleHeap, CommandManager& commandManager, const std::string& shaderFilePath)
	:
	mRootSignature(&rootSignature),
	mShaderVisibleHeap(&shaderVisibleHeap),
	mShaderNonVisibleHeap(&shaderNonVisibleHeap),
	mCommandManager(&commandManager),
	mUploadParams(nullptr)
{
	//パイプラインステートの初期化
	mPSO = ComputePipelineStateBuilder()
		.setRootSignature(mRootSignature)
		.setComputeShader(shaderFilePath)
		.build(device);

	//ビューを追加するディスクリプタヒープの領域を確保
	const int numVisibleDescriptors = 8;
	const int numNonVisibleDescriptors = 4;
	mShaderVisibleHeapRange = mShaderVisibleHeap->allocate(NumSlots(numVisibleDescriptors));
	mShaderNonVisibleHeapRange = mShaderNonVisibleHeap->allocate(NumSlots(numNonVisibleDescriptors));

}
