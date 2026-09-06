
#include "Memory/RWStructuredBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Command/Command.h"
#include "Memory/UploadBuffer.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/ReadBackBuffer.h"

RWStructuredBuffer::~RWStructuredBuffer() = default;

void RWStructuredBuffer::upload(void* srcData, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;

	if (!mUploadBuffer)
		mUploadBuffer = std::make_unique <UploadBuffer>(*mDevice, mSizeOfElement * mNumElements);

	mUploadBuffer->upload(srcData, 0, sizeInBytes);

	mDefaultBuffer->copyFromUploadBuffer(*mUploadBuffer.get());
}

void* RWStructuredBuffer::read()
{
	if (mDefaultBuffer == nullptr) return nullptr;

	if(!mReadBackBuffer)
		mReadBackBuffer = std::make_unique<ReadBackBuffer>(*mDevice, mCommandManager, mNumElements * mSizeOfElement);

	mReadBackBuffer->read(*mDefaultBuffer.get());

	return mReadBackBuffer->getCPUResource();
}

RWStructuredBuffer::RWStructuredBuffer(ID3D12Device& device, CommandManager& commandManager, UINT numElements, UINT sizeOfElement)
	:IRWStructuredBuffer(),
	mDevice(&device),
	mCommandManager(commandManager),
	mNumElements(numElements),
	mSizeOfElement(sizeOfElement)
{
	mDefaultBuffer = std::make_unique<LinearDefaultBuffer>(device, mCommandManager, numElements * sizeOfElement, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

ID3D12Resource* RWStructuredBuffer::getGPUResource()  const{
	return mDefaultBuffer->getGPUResource();
}
