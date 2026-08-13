#pragma once
#include "Graphic/Core/Definition.h"
#include <d3dx12.h>

class UploadHeap
{
public:
	UploadHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	~UploadHeap();

	void uploadData(void* sourceData, UINT SizeInBytes);
	ID3D12Resource* getGPUResource() { return mGPUResource.Get(); }

private:
	ID3D12GraphicsCommandList* mCommandList;
	ComPtr<ID3D12Resource> mGPUResource;
	void* mCPUResource;

};

