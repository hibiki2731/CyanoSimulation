#include "pch.h"
#include "Command/CommandListBuilder.h"

CommandListBuilder& CommandListBuilder::setNodeMask(UINT nodeMask) {
	mNodeMask = nodeMask;
	return *this;
}

CommandListBuilder& CommandListBuilder::setCommandListType(D3D12_COMMAND_LIST_TYPE& type) {
	mType = type;
	return *this;
}

CommandListBuilder& CommandListBuilder::setCommandAllocator(ComPtr<ID3D12CommandAllocator>& commandAllocator)
{
	mCommandAllocator = commandAllocator.Get();
	return *this;
}

CommandListBuilder& CommandListBuilder::setInitialPSO(ComPtr<ID3D12PipelineState>& pso)
{
	mPSO = pso.Get();
	return *this;
}

ComPtr<ID3D12GraphicsCommandList> CommandListBuilder::build(ID3D12Device& device) {
	//コマンドリストを初期化
	ComPtr<ID3D12GraphicsCommandList> commandList;

	//コマンドリストを作成
	HRESULT hr = device.CreateCommandList(mNodeMask, mType,
		mCommandAllocator, mPSO, IID_PPV_ARGS(commandList.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	return commandList;
}
