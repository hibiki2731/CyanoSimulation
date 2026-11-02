
#include "Memory/StructuredBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/UploadBuffer.h"
#include "Command/Command.h"
#include "Memory/RWStructuredBuffer.h"

StructuredBuffer::~StructuredBuffer() = default;

void StructuredBuffer::copyFrom(IStructuredBuffer& copySrc)
{
	StructuredBuffer& buffer = static_cast<StructuredBuffer&>(copySrc);
	mDefaultBuffer->copyFromDefaultBuffer(buffer.getDefaultBuffer());
}

void StructuredBuffer::copyFrom(IRWStructuredBuffer& copySrc)
{
	RWStructuredBuffer& buffer = static_cast<RWStructuredBuffer&>(copySrc);
	mDefaultBuffer->copyFromDefaultBuffer(buffer.getDefaultBuffer());
}

void StructuredBuffer::copyRegionFrom(IStructuredBuffer& copySrc, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;
	StructuredBuffer& buffer = static_cast<StructuredBuffer&>(copySrc);
	mDefaultBuffer->copyRegionFromDefaultBuffer(buffer.getDefaultBuffer(), srcOffset, dstOffset, sizeInBytes);
}

void StructuredBuffer::copyRegionFrom(IRWStructuredBuffer& copySrc, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;
	RWStructuredBuffer& buffer = static_cast<RWStructuredBuffer&>(copySrc);
	mDefaultBuffer->copyRegionFromDefaultBuffer(buffer.getDefaultBuffer(), srcOffset, dstOffset, sizeInBytes);
}


void StructuredBuffer::uploadToGPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;

	mUploadBuffer->upload(srcData, srcOffset, dstOffset, sizeInBytes);

	mDefaultBuffer->copyRegionFromUploadBuffer(*mUploadBuffer.get(), srcOffset, dstOffset, sizeInBytes);

}

void StructuredBuffer::uploadToCPU(void* srcData, UINT srcOffset, UINT dstOffset, UINT sizeInBytes)
{
	if (sizeInBytes == 0) return;

	mUploadBuffer->upload(srcData, srcOffset, dstOffset, sizeInBytes);

}

StructuredBuffer::StructuredBuffer(ID3D12Device& device, CommandManager& commandManager, UINT numElements, UINT sizeOfElement)
	:
	IStructuredBuffer(),
	mDevice(&device),
	mCommandManager(commandManager),
	mNumElements(numElements),
	mSizeOfElement(sizeOfElement)
{
	mDefaultBuffer = std::make_unique<LinearDefaultBuffer>(device, mCommandManager, mSizeOfElement * mNumElements, D3D12_RESOURCE_STATE_COMMON);
	mUploadBuffer = std::make_unique <UploadBuffer>(*mDevice, mNumElements * mSizeOfElement);
}

UploadBuffer& StructuredBuffer::getUploadBuffer() const
{
	return *mUploadBuffer;
}

LinearDefaultBuffer& StructuredBuffer::getDefaultBuffer() const
{
	return *mDefaultBuffer;
}
