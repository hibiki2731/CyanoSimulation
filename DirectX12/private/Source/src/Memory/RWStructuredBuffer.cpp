
#include "Memory/RWStructuredBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Command/Command.h"
#include "Memory/UploadBuffer.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/ReadBackBuffer.h"
#include "Memory/StructuredBuffer.h"

RWStructuredBuffer::~RWStructuredBuffer() = default;

void RWStructuredBuffer::copyFrom(IRWStructuredBuffer& copySrc)
{
	RWStructuredBuffer& buffer = static_cast<RWStructuredBuffer&>(copySrc);
	mDefaultBuffer->copyFromDefaultBuffer(buffer.getDefaultBuffer());
}

void RWStructuredBuffer::copyFrom(IStructuredBuffer& copySrc)
{
	StructuredBuffer& buffer = static_cast<StructuredBuffer&>(copySrc);
	mDefaultBuffer->copyFromDefaultBuffer(buffer.getDefaultBuffer());
}

void RWStructuredBuffer::uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;

	mUploadBuffer->upload(srcData, srcOffset, dstOffset, sizeInBytes);

	mDefaultBuffer->copyRegionFromUploadBuffer(*mUploadBuffer.get(), srcOffset, dstOffset, sizeInBytes);
}

void RWStructuredBuffer::uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;

	mUploadBuffer->upload(srcData, srcOffset, dstOffset, sizeInBytes);
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
	mUploadBuffer = std::make_unique <UploadBuffer>(*mDevice, mNumElements * mSizeOfElement);
}

LinearDefaultBuffer& RWStructuredBuffer::getDefaultBuffer() const {
	return *mDefaultBuffer;
}

UploadBuffer& RWStructuredBuffer::getUploadBuffer() const {
	return *mUploadBuffer;
}
