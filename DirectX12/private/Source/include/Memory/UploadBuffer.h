#pragma once
#include "graphic/core/definition.h"
#include <d3dx12.h>

class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device& device, UINT sizeInBytes);
	~UploadBuffer();

	void upload(void* sourceData, UINT offset, UINT SizeInBytes);
	ID3D12Resource* getGPUResource() { return mGPUResource.Get(); }
	void* getCPUResource() { return mCPUResource; }

private:
	D3D12_HEAP_PROPERTIES createHeapProperties();
	D3D12_RESOURCE_DESC createResourceDesc(int sizeInBytes); //リソースの詳細設定

	void createAndMapBuffer(ID3D12Device& device, D3D12_HEAP_PROPERTIES prop, D3D12_RESOURCE_DESC desc);
	ComPtr<ID3D12Resource> mGPUResource;
	void* mCPUResource;


};

