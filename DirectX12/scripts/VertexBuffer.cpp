#include "pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(ID3D12Device& device, const VertexBufferDescription& desc, const std::vector<float>& data)
	:
	mDesc(desc),
	mSizeInBytes(desc.numVertices * desc.numElementsPerVertex * sizeof(float)),
	mStrideInBytes(desc.numElementsPerVertex * sizeof(float))
{
	//入力されたdescとdataの整合性がとれているか確認
	assert(desc.numElementsPerVertex * desc.numVertices == data.size()
		&& "入力された頂点数、1頂点あたりの要素数とデータのサイズが一致しません");

	//バッファの確保
	createBuf(device);

	//データのコピー
	setData(data);

	//ビューの作成
	createView();
}

void VertexBuffer::createBuf(ID3D12Device& device)
{
	
	auto prop = createHeapProperties();
	auto desc = createResourceDesc();

	HRESULT hr = device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mVertexBuffer.ReleaseAndGetAddressOf())
	);

	assert(SUCCEEDED(hr));
}

D3D12_HEAP_PROPERTIES VertexBuffer::createHeapProperties()
{
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD; //CPUからGPUへ書き込み
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}

D3D12_RESOURCE_DESC VertexBuffer::createResourceDesc()
{
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

void VertexBuffer::setData(const std::vector<float>& src)
{
	UINT8* mappedBuf; //コピー先のアドレス
	HRESULT hr = mVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
	assert(SUCCEEDED(hr));

	memcpy(mappedBuf, src.data(), mSizeInBytes);
	mVertexBuffer->Unmap(0, nullptr);
}

void VertexBuffer::createView()
{
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = mSizeInBytes;//全バイト数
	mVertexBufferView.StrideInBytes = mStrideInBytes;//１頂点のバイト数
}
