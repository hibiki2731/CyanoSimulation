#pragma once
#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
class ReadBackBuffer
{
public:
	//要素のサイズと数を入力
	ReadBackBuffer(ID3D12Device& device, class Command& copyCommand, UINT sizeInBytes);
	~ReadBackBuffer();

	void read(class LinearDefaultBuffer& readSrc);
	void* getCPUResource() const;
private:
	D3D12_RESOURCE_DESC createResourceDesc();
	D3D12_HEAP_PROPERTIES createHeapProperties();
	void createFence(ID3D12Device& device);
	void createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop);

	ComPtr<ID3D12Resource> mGPUResource;
	class Command& mCopyCommand;
	void* mCPUResource;
	UINT mSizeInBytes;
	std::unique_ptr<class Fence> mFence;
};


