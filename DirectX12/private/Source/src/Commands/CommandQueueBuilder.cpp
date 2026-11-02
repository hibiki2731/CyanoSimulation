#include "pch.h"
#include "Command/CommandQueueBuilder.h"

CommandQueueBuilder& CommandQueueBuilder::setCommandListType(D3D12_COMMAND_LIST_TYPE& type) {
	mDesc.Type = type;
	return *this;
}

CommandQueueBuilder& CommandQueueBuilder::setFlag(D3D12_COMMAND_QUEUE_FLAGS& flag)
{
	mDesc.Flags = flag;
	return *this;
}

CommandQueueBuilder& CommandQueueBuilder::setNodeMask(UINT nodeMask){
	mDesc.NodeMask = nodeMask;
	return *this;
}

ComPtr<ID3D12CommandQueue> CommandQueueBuilder::build(ID3D12Device& device) {
	//コマンドキューの初期化
	ComPtr<ID3D12CommandQueue> commandQueue;

	//コマンドキューを作成
	HRESULT hr = device.CreateCommandQueue(&mDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return commandQueue;
}
