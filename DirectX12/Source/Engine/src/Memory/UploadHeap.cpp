#include "Memory/UploadHeap.h"

UploadHeap::UploadHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	:mCommandList(commandList),
	mGPUResource(nullptr),
	mCPUResource(nullptr)
{
}

UploadHeap::~UploadHeap()
{
}

void UploadHeap::uploadData(void* sourceData, UINT SizeInBytes)
{
}
