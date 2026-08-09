#pragma once
#include <vector>
#include "Definition.h"
#include "d3dx12.h"
class RWStructuredBuffer
{
public:
	//要素のサイズと数を入力
	RWStructuredBuffer(ID3D12Device& device, int sizeOfElement, int numElement);
	~RWStructuredBuffer();

	void copyData(const void* resource, const size_t resourceSize);
	ID3D12Resource* getBufferOnGPU() const;
	void* getBufferOnCPU() const;
	const int getSizeOfElement() const { return mSizeOfElement; }
	const int getNumElements() const { return mNumElement; }

private:
	D3D12_RESOURCE_DESC getResourceDesc();
	D3D12_HEAP_PROPERTIES getHeapProperties();
	void createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop);

	const int mSizeOfElement;
	const int mNumElement;
	ComPtr<ID3D12Resource> mBuffersOnGPU;
	void* mBuffersOnCPU;
};

