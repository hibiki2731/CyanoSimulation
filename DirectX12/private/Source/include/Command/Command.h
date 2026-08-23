#pragma once
#include <d3dx12.h>
using Microsoft::WRL::ComPtr;

class Command
{
public:
	Command(ID3D12Device& device, UINT commandNum);

	ComPtr<ID3D12GraphicsCommandList>& getList() {
		return mCommandList;
	};

	ComPtr<ID3D12CommandQueue>& getQueue() {
		return mCommandQueue;
	};

	ComPtr<ID3D12CommandAllocator>& getAllocator(UINT index = 0) {
		assert(index < static_cast<UINT>(mCommandAllocators.size()));
		return mCommandAllocators[index];
	};

private:
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	std::vector<ComPtr<ID3D12CommandAllocator>> mCommandAllocators;
};

