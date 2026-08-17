#include "pch.h"
#include "Graphic/Core/CommandAllocatorBuilder.h"

CommandAllocatorBuilder& CommandAllocatorBuilder::setCommandListType(D3D12_COMMAND_LIST_TYPE& type)
{
	mType = type;
	return *this;
}

ComPtr<ID3D12CommandAllocator> CommandAllocatorBuilder::build(ID3D12Device& device) {
	//コマンドアロケータの初期化
	ComPtr<ID3D12CommandAllocator> commandAllocator;

	//コマンドアロケータを作成
	HRESULT hr = device.CreateCommandAllocator(mType,
		IID_PPV_ARGS(commandAllocator.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return commandAllocator;
}
