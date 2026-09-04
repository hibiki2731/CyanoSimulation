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

class CommandManager
{
public:
	CommandManager(ID3D12Device& device, UINT frameCount);

	ComPtr<ID3D12CommandAllocator>& getComputeCommandAllocator() {
		return mComputeCommandAllocator;
	}

	ComPtr<ID3D12CommandAllocator>& getGraphicsCommandAllocator(UINT frame = 0) {
		assert(frame < static_cast<UINT>(mGraphicsCommandAllocators.size()));
		return mGraphicsCommandAllocators[frame];
	}

	ComPtr<ID3D12GraphicsCommandList>& getComputeCommandList() {
		return mComputeCommandList;
	}

	ComPtr<ID3D12GraphicsCommandList>& getGraphicsCommandList() {
		return mGraphicsCommandList;
	}

	ComPtr<ID3D12CommandQueue>& getCommandQueue() {
		return mCommandQueue;
	}

private:
	std::vector<ComPtr<ID3D12CommandAllocator>> mGraphicsCommandAllocators;
	ComPtr<ID3D12CommandAllocator> mComputeCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mGraphicsCommandList;
	ComPtr<ID3D12GraphicsCommandList> mComputeCommandList;
	ComPtr<ID3D12CommandQueue> mCommandQueue;

};
