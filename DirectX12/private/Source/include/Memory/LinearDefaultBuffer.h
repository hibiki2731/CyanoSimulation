#pragma once
#include "graphic/core/definition.h"
#include <d3dx12.h>

class LinearDefaultBuffer
{

public:
	LinearDefaultBuffer(ID3D12Device& device, ID3D12GraphicsCommandList& commandList, UINT sizeInBytes, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_NONE);

	void copyFromUploadBuffer(class UploadBuffer& copySrc);

	ID3D12Resource* getGPUResource() { return mGPUResource.Get(); }
	D3D12_RESOURCE_STATES getResourceState() { return mState; }

private:

	D3D12_HEAP_PROPERTIES createHeapProperties();
	D3D12_RESOURCE_DESC createResourceDesc(int sizeInBytes, D3D12_RESOURCE_FLAGS flags); //リソースの詳細設定

	void createBuffer(ID3D12Device& device, D3D12_HEAP_PROPERTIES prop, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES state);

	ID3D12GraphicsCommandList* mCommandList;
	ComPtr<ID3D12Resource> mGPUResource;
	UINT mSizeInBytes;
	D3D12_RESOURCE_STATES mState;
};

