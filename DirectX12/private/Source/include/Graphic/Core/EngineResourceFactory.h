#pragma once
#include <d3dx12.h>
#include <vector>
#include <memory>
using Microsoft::WRL::ComPtr;

class EngineResourceFactory
{
public:
	std::shared_ptr<class StructuredBuffer> createStructuredBuffer(UINT numElements, UINT sizeOfElement);
	std::shared_ptr<class RWStructuredBuffer> createRWStructuredBuffer(UINT numElements, UINT sizeOfElement);

private:
	friend class Game;
	EngineResourceFactory(ID3D12Device& device, class Command& graphicsCommand, class Command& copyCommand);
	ID3D12Device& mDevice;
	class Command& mGraphicsCommand;
	class Command& mCopyCommand;
};

