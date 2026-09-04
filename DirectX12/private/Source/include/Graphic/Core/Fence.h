#pragma once
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include <vector>

class Fence
{
public:
	Fence(ID3D12Device& device, const int frameCount = 1);
	~Fence() = default;

	void waitGPU(ID3D12CommandQueue& commandQueue);
	void waitCompleteNextFrame(ID3D12CommandQueue& commandQueue, const int currentFrame, const int nextFrame);

private:
	void prepareFence(ID3D12Device& device, const int frameCount);
	
	ComPtr<ID3D12Fence> mFence;
	HANDLE mFenceEvent;
	UINT64 mFenceValue;
	std::vector<UINT64> mFrameFenceValues;
};

