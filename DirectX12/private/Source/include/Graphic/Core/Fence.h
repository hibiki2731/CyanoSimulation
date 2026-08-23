#pragma once
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include <vector>

class Fence
{
public:
	Fence(ID3D12Device& device, ID3D12CommandQueue& commandQueue, const int frameCount = 1);
	~Fence() = default;

	void waitGPU();
	void waitCompleteNextFrame(const int currentFrame, const int nextFrame);

private:
	void prepareFence(ID3D12Device& device, const int frameCount);
	
	ComPtr<ID3D12Fence> mFence;
	HANDLE mFenceEvent;
	UINT64 mFenceValue;
	std::vector<UINT64> mFrameFenceValues;
	ID3D12CommandQueue& mCommandQueue;
};

