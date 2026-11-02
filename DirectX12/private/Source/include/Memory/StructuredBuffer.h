#pragma once
#include <vector>
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>
#include "Memory/IStructuredBuffer.h"

class StructuredBuffer : public IStructuredBuffer
{
public:
	~StructuredBuffer();

	void copyFrom(IStructuredBuffer& copySrc) override;
	void copyFrom(class IRWStructuredBuffer& copySrc) override;

	void uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) override;
	void uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes) override;

	class UploadBuffer& getUploadBuffer() const;
	class LinearDefaultBuffer& getDefaultBuffer() const;
	const UINT getNumElements() const { return mNumElements; }
	const UINT getSizeOfElement() const { return mSizeOfElement; }

private:
	friend class EngineResourceFactory;
	//要素のサイズと数を入力
	StructuredBuffer(ID3D12Device& device, class CommandManager& commandManager, UINT numElements, UINT sizeOfElement);

	std::unique_ptr<class UploadBuffer> mUploadBuffer;
	std::unique_ptr<class LinearDefaultBuffer> mDefaultBuffer;
	ID3D12Device* mDevice;
	class CommandManager& mCommandManager;
	UINT mNumElements;
	UINT mSizeOfElement;

};


