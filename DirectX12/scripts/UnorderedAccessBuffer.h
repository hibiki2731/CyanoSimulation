#pragma once
class UnorderedAccessBuffer
{
public:
	//要素のサイズと数を入力
	UnorderedAccessBuffer(class Graphic& graphic, int sizeOfElement, int numElement);

private:
	void init();
	const int mSizeOfElement;
	const int mNumElement;
	std::vector<ComPtr<ID3D12Resource>> mBuffersOnGPU;
	std::vector<void*> mBuffersOnCPU;
};

