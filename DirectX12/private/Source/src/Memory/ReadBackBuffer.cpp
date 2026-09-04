
#include "Memory/ReadBackBuffer.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Graphic/Core/Fence.h"
#include "Command/Command.h"

ReadBackBuffer::ReadBackBuffer(ID3D12Device& device, CommandManager& commandManager, UINT sizeInBytes)
	:mSizeInBytes(sizeInBytes),
	mCommandManager(commandManager)
{
	//バッファーの詳細設定
	auto desc = createResourceDesc();
	//ヒープ設定
	auto prop = createHeapProperties();

	//バッファーの作成とマッピング
	createAndMapBuffers(device, desc, prop);

	//フェンスの作成
	createFence(device);
}

ReadBackBuffer::~ReadBackBuffer()
{
	//アンマップ
	CD3DX12_RANGE range(0, 0);
	mGPUResource->Unmap(0, &range);
}

void ReadBackBuffer::read(LinearDefaultBuffer& readSrc)
{
	//コピー元のバッファのステートをCOPY_SOURCEに変更
	D3D12_RESOURCE_BARRIER barrierToCopySrc = CD3DX12_RESOURCE_BARRIER::Transition(
		readSrc.getGPUResource(),
		readSrc.getResourceState(),
		D3D12_RESOURCE_STATE_COPY_SOURCE
	);
	mCommandManager.getComputeCommandList()->ResourceBarrier(1, &barrierToCopySrc);

	//コピーの実行
	mCommandManager.getComputeCommandList()->CopyResource(mGPUResource.Get(), readSrc.getGPUResource());

	//コピー元のステートを元の状態に戻す
	D3D12_RESOURCE_BARRIER barrierToRestore = CD3DX12_RESOURCE_BARRIER::Transition(
		readSrc.getGPUResource(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		readSrc.getResourceState()
	);
	mCommandManager.getComputeCommandList()->ResourceBarrier(1, &barrierToRestore);

	//GPUへ命令を送信
	mCommandManager.getComputeCommandList()->Close();
	ID3D12CommandList* commandLists[] = { mCommandManager.getComputeCommandList().Get() };
	mCommandManager.getCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	//GPUがコピー完了するまでCPUを待機
	mFence->waitGPU(*mCommandManager.getCommandQueue().Get());

	//コピー用コマンドの初期化
	mCommandManager.getComputeCommandAllocator()->Reset();
	mCommandManager.getComputeCommandList()->Reset(mCommandManager.getComputeCommandAllocator().Get(), nullptr);
}

void* ReadBackBuffer::getCPUResource() const
{
	return mCPUResource;
}

D3D12_RESOURCE_DESC ReadBackBuffer::createResourceDesc()
{
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(mSizeInBytes);
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	return desc;
}

D3D12_HEAP_PROPERTIES ReadBackBuffer::createHeapProperties()
{
	D3D12_HEAP_PROPERTIES prop{};
	prop.Type = D3D12_HEAP_TYPE_READBACK;		//ヒープ設定を手動で行う
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}

void ReadBackBuffer::createFence(ID3D12Device& device)
{
	//フェンスを作成
	mFence = std::make_unique<Fence>(device, 1);
}

void ReadBackBuffer::createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop)
{
	//バッファーの確保
	HRESULT hr = device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(mGPUResource.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));

	//GPUに確保したバッファをCPUからアクセス可能なアドレス空間にマッピングする
	CD3DX12_RANGE readRange(0, mSizeInBytes); //読み取り範囲は全体
	mGPUResource->Map(0, &readRange, reinterpret_cast<void**>(&mCPUResource));

}

