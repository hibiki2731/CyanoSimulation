#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include "Memory/IRWStructuredBuffer.h"
class RWStructuredBuffer : public IRWStructuredBuffer
{
public:
	~RWStructuredBuffer();

	void copyFrom(IRWStructuredBuffer& copySrc) override;
	void copyFrom(class IStructuredBuffer& copySrc) override;
	void uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) override;
	void uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) override;
	void* read() override;

	class LinearDefaultBuffer& getDefaultBuffer() const;
	class UploadBuffer& getUploadBuffer() const;
	const UINT getNumElements() const { return mNumElements; }
	const UINT getSizeOfElement() const { return mSizeOfElement; }

private:
	friend class EngineResourceFactory;
	RWStructuredBuffer(ID3D12Device& device, class CommandManager& commandManager, UINT numElements, UINT sizeOfElement);

	std::unique_ptr<class UploadBuffer> mUploadBuffer;
	std::unique_ptr<class LinearDefaultBuffer> mDefaultBuffer;
	std::unique_ptr<class ReadBackBuffer> mReadBackBuffer;
	ID3D12Device* mDevice;
	class CommandManager& mCommandManager;
	const UINT mNumElements;
	const UINT mSizeOfElement;

};

