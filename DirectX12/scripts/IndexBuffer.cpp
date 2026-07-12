#include "pch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(ID3D12Device& device, const UINT sizeInBytes, const std::vector<UINT16>& data) 
	:mSizeInBytes(sizeInBytes)
{
	//バッファの確保
	createBuf(device);

	//データのコピー
	setData(data);

	//ビューの作成
	createView();
}

void IndexBuffer::createBuf(ID3D12Device& device) {

	auto prop = createHeapProperties();
	auto desc = createResourceDesc();

	HRESULT hr = device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mIndexBuffer.ReleaseAndGetAddressOf())
	);

	assert(SUCCEEDED(hr));
	
}


D3D12_HEAP_PROPERTIES IndexBuffer::createHeapProperties() {

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD; //CPUからGPUへ書き込み
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}


D3D12_RESOURCE_DESC IndexBuffer::createResourceDesc() {

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //一次元配列
	desc.Alignment = 0;
	desc.Width = mSizeInBytes; //バッファのサイズ
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	return desc;
}

void IndexBuffer::setData(const std::vector<UINT16>& src) {

	UINT8* mappedBuf; //コピー先のアドレス
	HRESULT hr = mIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
	assert(SUCCEEDED(hr));

	memcpy(mappedBuf, src.data(), mSizeInBytes);
	mIndexBuffer->Unmap(0, nullptr);
}


void IndexBuffer::createView() {
	//インデックスバッファービューを作る
	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = mSizeInBytes;//全バイト数
	mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;//UINT16
}
