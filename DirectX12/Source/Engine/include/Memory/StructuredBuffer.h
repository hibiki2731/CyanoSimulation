#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include "External/DirectX//d3dx12.h"

class StructuredBuffer
{
public:
	//要素のサイズと数を入力
	StructuredBuffer(ID3D12Device& device, int sizeOfElement, int numElement);
	~StructuredBuffer();

	void setData(ID3D12GraphicsCommandList& commandList, const void* resource);
	ID3D12Resource* getBufferOnGPU() const;
	const int getSizeOfElement() const { return mSizeOfElement; }
	const int getNumElements() const { return mNumElement; }

private:
	D3D12_RESOURCE_DESC getResourceDesc();
	D3D12_HEAP_PROPERTIES getHeapProperties();
	void createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop);

	const int mSizeOfElement;
	const int mNumElement;
	ComPtr<ID3D12Resource> mBuffersOnGPU;
	ID3D12Device& mDevice;
};


