
#include "Memory/StructuredBuffer.h"
#include "Graphic/Core/Graphic.h"
#include "Memory/LinearDefaultBuffer.h"
#include "Memory/UploadBuffer.h"
#include "Command/Command.h"

StructuredBuffer::~StructuredBuffer() = default;

void StructuredBuffer::upload(void* srcData, UINT sizeInBytes)
{
	if (mUploadBuffer) mUploadBuffer.reset();

	mUploadBuffer = std::make_unique <UploadBuffer>(*mDevice, sizeInBytes);
	mUploadBuffer->upload(srcData, 0, sizeInBytes);

	mDefaultBuffer->copyFromUploadBuffer(*mUploadBuffer.get());
}

StructuredBuffer::StructuredBuffer(ID3D12Device& device, class Command& copyCommand, UINT numElements, UINT sizeOfElement)
	:
	IStructuredBuffer(),
	mDevice(&device),
	mCopyCommand(&copyCommand),
	mNumElements(numElements),
	mSizeOfElement(sizeOfElement)
{
	mDefaultBuffer = std::make_unique<LinearDefaultBuffer>(device, *copyCommand.getList().Get(), mSizeOfElement * mNumElements, D3D12_RESOURCE_STATE_COMMON);
}

ID3D12Resource* StructuredBuffer::getGPUResource() const{
	return mDefaultBuffer->getGPUResource();
}
