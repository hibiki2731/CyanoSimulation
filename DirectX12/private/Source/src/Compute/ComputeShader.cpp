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
	list->SetComputeRoot32BitConstants(0, 4, mUploadPointer, 0);

	//ディスクリプタテーブルの設定
	list->SetComputeRootDescriptorTable(1, mShaderVisibleHeap->getGPUHandle(mShaderVisibleHeapRange->getIndex(0)));
	list->SetComputeRootDescriptorTable(2, mShaderVisibleHeap->getGPUHandle(mShaderVisibleHeapRange->getIndex(4)));


	list->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

}

void ComputeShader::setStructuredBuffer(IStructuredBuffer& buffer, UINT position)
{
	assert(position < mNumSBuffers);
	mShaderVisibleHeap->addSRV(buffer, mShaderVisibleHeapRange->getIndex(position));
}

void ComputeShader::setRWStructuredBuffer(IRWStructuredBuffer& buffer, UINT position)
{
	assert(position < mNumRWBuffers);
	mShaderVisibleHeap->addUAV(buffer, mShaderVisibleHeapRange->getIndex(mNumSBuffers + position));

	//クリアの準備
	mShaderVisibleHeap->addRawBuffer(buffer, mShaderVisibleHeapRange->getIndex(mNumSBuffers + mNumRWBuffers + position));
	mShaderNonVisibleHeap->addRawBuffer(buffer, mShaderNonVisibleHeapRange->getIndex(position));
	mRegisteredRWBuffers[position] = static_cast<RWStructuredBuffer*>(& buffer);
}
void ComputeShader::setRootConstants(void* dataSrc)
{
	mUploadPointer = dataSrc;
}

void ComputeShader::waitWriteBuffer(UINT position)
{
	assert(position < mNumRWBuffers);
	//書き込みが終わるまで、リソースの使用を待機させる
	std::array<D3D12_RESOURCE_BARRIER, 1> barrier = {};
	barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier[0].UAV.pResource = mRegisteredRWBuffers[position]->getGPUResource();
	mCommandManager->getComputeCommandList()->ResourceBarrier(1, barrier.data());
}

void ComputeShader::clearRWStructuredBuffer(UINT position)
{

	auto& list = mCommandManager->getComputeCommandList();
	//ディスクリプタヒープをセット
	list->SetDescriptorHeaps(1, mShaderVisibleHeap->getAddress());

	assert(position < mNumRWBuffers);
    static UINT clearValue[4] = { 0, 0, 0, 0 };
    mCommandManager->getComputeCommandList()->ClearUnorderedAccessViewUint(
        mShaderVisibleHeap->getGPUHandle(mShaderVisibleHeapRange->getIndex(mNumSBuffers * mNumRWBuffers + position)),                  // シェーダー可視ヒープ上のGPUハンドル
        mShaderNonVisibleHeap->getCPUHandle(mShaderNonVisibleHeapRange->getIndex(position)),  // 同じビューを指す、非シェーダー可視ヒープ上のCPUハンドル
        mRegisteredRWBuffers[position]->getGPUResource(),
        clearValue, 0, nullptr
    );
}

ComputeShader::ComputeShader(ID3D12Device& device, ID3D12RootSignature& rootSignature, DescriptorHeap& shaderVisibleHeap, DescriptorHeap& shaderNonVisibleHeap, CommandManager& commandManager, const std::string& shaderFilePath, ComputeShaderFormat format)
	:
	mRootSignature(&rootSignature),
	mShaderVisibleHeap(&shaderVisibleHeap),
	mShaderNonVisibleHeap(&shaderNonVisibleHeap),
	mCommandManager(&commandManager),
	mUploadPointer(nullptr),
	mRegisteredRWBuffers()
{
	//パイプラインステートの初期化
	mPSO = ComputePipelineStateBuilder()
		.setRootSignature(mRootSignature)
		.setComputeShader(shaderFilePath)
		.build(device);

	//フォーマットの確認
	switch (format) {
	case ComputeShaderFormat::S4_RW4:
		mNumSBuffers = 4;
		mNumRWBuffers = 4;
		break;

	case ComputeShaderFormat::RW_12:
		mNumSBuffers = 0;
		mNumRWBuffers = 12;
		break;
	default:
		mNumRWBuffers = 0;
		mNumSBuffers = 0;
	}
	mRegisteredRWBuffers.resize(mNumRWBuffers);

	//ビューを追加するディスクリプタヒープの領域を確保
	mShaderVisibleHeapRange = mShaderVisibleHeap->allocate(NumSlots(mNumSBuffers + mNumRWBuffers * 2));	//クリア用のビューも確保
	mShaderNonVisibleHeapRange = mShaderNonVisibleHeap->allocate(NumSlots(mNumRWBuffers));
}
