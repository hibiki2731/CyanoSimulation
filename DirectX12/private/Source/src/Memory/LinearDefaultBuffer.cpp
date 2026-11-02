#include "pch.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/UploadBuffer.h"
#include "Command/Command.h"

void LinearDefaultBuffer::copyFromUploadBuffer(UploadBuffer& copySrc)
{
	//コピー先のバッファのステートをCOPY_DESTに変更
	D3D12_RESOURCE_BARRIER barrierToCopyDesc[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			mState,
			D3D12_RESOURCE_STATE_COPY_DEST
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		)
	};

	auto& list = mCommandManager.getComputeCommandList();

	list->ResourceBarrier(2, barrierToCopyDesc);

	//リソースのコピー
	list->CopyResource(mGPUResource.Get(), copySrc.getGPUResource());

	//本来のステートに戻す
	D3D12_RESOURCE_BARRIER barrierToUsage[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			mState
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_GENERIC_READ
		)
	};
	list->ResourceBarrier(2, barrierToUsage);
}

void LinearDefaultBuffer::copyFromDefaultBuffer(LinearDefaultBuffer& copySrc)
{
	//コピー先のバッファのステートをCOPY_DESTに変更
	D3D12_RESOURCE_BARRIER barrierToCopyDesc[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			mState,
			D3D12_RESOURCE_STATE_COPY_DEST
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			copySrc.getResourceState(),
			D3D12_RESOURCE_STATE_COPY_SOURCE
		)
	};
	auto& list = mCommandManager.getComputeCommandList();

	list->ResourceBarrier(2, barrierToCopyDesc);

	//リソースのコピー
	list->CopyResource(mGPUResource.Get(), copySrc.getGPUResource());

	//本来のステートに戻す
	D3D12_RESOURCE_BARRIER barrierToUsage[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			mState
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			copySrc.getResourceState()
		)
	};
	list->ResourceBarrier(2, barrierToUsage);
}

void LinearDefaultBuffer::copyRegionFromUploadBuffer(UploadBuffer& copySrc, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	//コピー先のバッファのステートをCOPY_DESTに変更
	D3D12_RESOURCE_BARRIER barrierToCopyDesc[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			mState,
			D3D12_RESOURCE_STATE_COPY_DEST
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		)
	};

	auto& list = mCommandManager.getComputeCommandList();

	list->ResourceBarrier(2, barrierToCopyDesc);

	//リソースのコピー
	list->CopyBufferRegion(
		mGPUResource.Get(),
		dstOffset,
		copySrc.getGPUResource(),
		srcOffset,
		sizeInBytes
	);

	//本来のステートに戻す
	D3D12_RESOURCE_BARRIER barrierToUsage[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			mState
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_GENERIC_READ
		)
	};
	list->ResourceBarrier(2, barrierToUsage);
}

void LinearDefaultBuffer::copyRegionFromDefaultBuffer(LinearDefaultBuffer& copySrc, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	//コピー先のバッファのステートをCOPY_DESTに変更
	D3D12_RESOURCE_BARRIER barrierToCopyDesc[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			mState,
			D3D12_RESOURCE_STATE_COPY_DEST
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			copySrc.getResourceState(),
			D3D12_RESOURCE_STATE_COPY_SOURCE
		)
	};
	auto& list = mCommandManager.getComputeCommandList();

	list->ResourceBarrier(2, barrierToCopyDesc);

	//リソースのコピー
	list->CopyBufferRegion(
		mGPUResource.Get(),
		dstOffset,
		copySrc.getGPUResource(),
		srcOffset,
		sizeInBytes
	);

	//本来のステートに戻す
	D3D12_RESOURCE_BARRIER barrierToUsage[2] = 
	{ 
		CD3DX12_RESOURCE_BARRIER::Transition(
			mGPUResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			mState
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			copySrc.getGPUResource(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			copySrc.getResourceState()
		)
	};
	list->ResourceBarrier(2, barrierToUsage);
}

LinearDefaultBuffer::LinearDefaultBuffer(ID3D12Device& device, CommandManager& commandManager, UINT sizeInBytes, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flag)
	:mCommandManager(commandManager),
	mSizeInBytes(sizeInBytes),
	mState(state)
{
	auto prop = createHeapProperties();
	auto desc = createResourceDesc(sizeInBytes, flag);

	createBuffer(device, prop, desc, state);

}

D3D12_HEAP_PROPERTIES LinearDefaultBuffer::createHeapProperties() 
{
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}

D3D12_RESOURCE_DESC LinearDefaultBuffer::createResourceDesc(int sizeInBytes, D3D12_RESOURCE_FLAGS flag) 
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //一次元配列
	desc.Alignment = 0;
	desc.Width = sizeInBytes; //バッファのサイズ
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = flag;

	return desc;
}

void LinearDefaultBuffer::createBuffer(ID3D12Device& device, D3D12_HEAP_PROPERTIES prop, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES state)
{
	HRESULT hr = device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		state,
		nullptr,
		IID_PPV_ARGS(mGPUResource.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));

}
