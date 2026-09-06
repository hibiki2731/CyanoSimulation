#include "Memory/UploadBuffer.h"


UploadBuffer::UploadBuffer(ID3D12Device& device, UINT sizeInBytes)
	:mCPUResource(nullptr)
{
	if (sizeInBytes == 0) return;

	auto prop = createHeapProperties();
	auto desc = createResourceDesc(sizeInBytes);

	createAndMapBuffer(device, prop, desc);

}

UploadBuffer::~UploadBuffer()
{
	mGPUResource->Unmap(0, nullptr);

}

void UploadBuffer::upload(void* sourceData, UINT offset, UINT sizeInBytes)
{
	assert(sourceData != nullptr);

	memcpy(static_cast<UINT*>(mCPUResource) + offset, sourceData, sizeInBytes);
}


D3D12_HEAP_PROPERTIES UploadBuffer::createHeapProperties() {

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD; //CPUからGPUへ書き込み
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}

D3D12_RESOURCE_DESC UploadBuffer::createResourceDesc(int sizeInBytes)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //一次元配列
	desc.Alignment = 0;
	desc.Width = sizeInBytes; //バッファのサイズ
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return desc;

}

void UploadBuffer::createAndMapBuffer(ID3D12Device& device, D3D12_HEAP_PROPERTIES prop, D3D12_RESOURCE_DESC desc)
{
	HRESULT hr = device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, //UPLOADヒープの場合はGENERIC_READでなくてはいけない
		nullptr,
		IID_PPV_ARGS(mGPUResource.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));

	//確保したバッファをCPUからアクセス可能なアドレス空間にマッピングする
	CD3DX12_RANGE readRange(0, 0); //CPUからGPUへの書き込みのみなので、読み取り範囲は0
	hr = mGPUResource->Map(0, &readRange, reinterpret_cast<void**>(&mCPUResource));
	assert(SUCCEEDED(hr));

}
