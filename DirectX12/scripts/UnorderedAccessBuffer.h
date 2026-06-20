#pragma once
class UnorderedAccessBuffer
{
public:
	//要素のサイズと数を入力
	UnorderedAccessBuffer(class Graphic& graphic, int sizeOfElement, int numElement);

	const ID3D12Resource& getBufferOnGPU() const;
	const void* getBufferOnCPU() const;

private:
	D3D12_RESOURCE_DESC getResourceDesc();
	D3D12_HEAP_PROPERTIES getHeapProperties();
	void createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop);

	const int mSizeOfElement;
	const int mNumElement;
	std::vector<ComPtr<ID3D12Resource>> mBuffersOnGPU;
	std::vector<void*> mBuffersOnCPU;
};

