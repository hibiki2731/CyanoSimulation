#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include "Memory/IRWStructuredBuffer.h"
class RWStructuredBuffer : public IRWStructuredBuffer
{
public:
	~RWStructuredBuffer();

	void upload(void* srcData, UINT sizeInBytes) override;
	void* read() override;

	ID3D12Resource* getGPUResource() const;
	const UINT getNumElements() const { return mNumElements; }
	const UINT getSizeOfElement() const { return mSizeOfElement; }

private:
	friend class EngineResourceFactory;
	RWStructuredBuffer(ID3D12Device& device, class Command& CompyCommand, UINT numElements, UINT sizeOfElement);

	std::unique_ptr<class UploadBuffer> mUploadBuffer;
	std::unique_ptr<class LinearDefaultBuffer> mDefaultBuffer;
	ID3D12Device* mDevice;
	class Command* mCopyCommand;
	UINT mNumElements;
	UINT mSizeOfElement;

};

