#pragma once
#include <vector>
#include "Definition.h"
#include "d3dx12.h"
class UnorderedAccessBuffer
{
public:
	//要素のサイズと数を入力
	UnorderedAccessBuffer(ID3D12Device& device, int sizeOfElement, int numElement);
	~UnorderedAccessBuffer();

	void copyData(const void* resource, const size_t resourceSize, const int frame);
	ID3D12Resource* getBufferOnGPU(const int frame) const;
	void* getBufferOnCPU(const int frame) const;
	const int getSizeOfElement() const { return mSizeOfElement; }
	const int getNumElements() const { return mNumElement; }

private:
	D3D12_RESOURCE_DESC getResourceDesc();
	D3D12_HEAP_PROPERTIES getHeapProperties();
	void createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop);

	const int mSizeOfElement;
	const int mNumElement;
	std::vector<ComPtr<ID3D12Resource>> mBuffersOnGPU;
	std::vector<void*> mBuffersOnCPU;
};

