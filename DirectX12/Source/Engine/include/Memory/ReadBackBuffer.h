#pragma once
#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
class ReadBackBuffer
{
public:
	//要素のサイズと数を入力
	ReadBackBuffer(ID3D12Device& device, int sizeOfElement, int numElement);
	~ReadBackBuffer();

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


