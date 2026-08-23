#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include "Memory/IStructuredBuffer.h"

class StructuredBuffer : public IStructuredBuffer
{
public:
	~StructuredBuffer();
	void upload(void* srcData, UINT sizeInBytes) override;

	ID3D12Resource* getGPUResource() const;
	const UINT getNumElements() const { return mNumElements; }
	const UINT getSizeOfElement() const { return mSizeOfElement; }

private:
	friend class EngineResourceFactory;
	//要素のサイズと数を入力
	StructuredBuffer(ID3D12Device& device, class Command& copyCommand, UINT numElements, UINT sizeOfElement);

	std::unique_ptr<class LinearDefaultBuffer> mDefaultBuffer;
	ID3D12Device& mDevice;
	class Command& mCopyCommand;
	UINT mNumElements;
	UINT mSizeOfElement;

};


