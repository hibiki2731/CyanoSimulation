
#include "Memory/StructuredBuffer.h"
#include "Graphic/Core/Graphic.h"

StructuredBuffer::StructuredBuffer(ID3D12Device& device, int sizeOfElement, int numElement)
	:mSizeOfElement(sizeOfElement)
	,mNumElement(numElement)
	,mDevice(device)
{
	if (sizeOfElement <= 0 || numElement <= 0) {
		assert(false && "UnorderedAccessBufferの要素サイズと要素数は1以上である必要があります。");
		return;
	}

	//バッファーの詳細設定
	auto desc = getResourceDesc();
	//ヒープ設定
	auto prop = getHeapProperties();

	//バッファーの作成とマッピング
	createAndMapBuffers(device, desc, prop);
}

StructuredBuffer::~StructuredBuffer()
{
	//アンマップ
	CD3DX12_RANGE range(0, 0);
	mBuffersOnGPU->Unmap(0, &range);
}

void StructuredBuffer::setData(ID3D12GraphicsCommandList& commandList, const void* resource)
{
	//アップロード用中間バッファをつくり、生データをコピー
	ComPtr<ID3D12Resource> uploadBuf;
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = mSizeOfElement * mNumElement;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN; // ストラクチャードバッファは UNKNOWN に設定
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		HRESULT hr = mDevice.CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			nullptr,
			IID_PPV_ARGS(&uploadBuf));
		assert(SUCCEEDED(hr));

		//生データをuploadbuffに一旦コピー
		uint8_t* mapBuf = nullptr;
		hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//マップ
		memcpy(mapBuf, resource, mNumElement * mSizeOfElement);
		uploadBuf->Unmap(0, nullptr);//アンマップ
	}

	//structuredBufferへコピー
	{
		//structuredBufferをCOPY_DESTへ
		D3D12_RESOURCE_BARRIER toCopyDest = {};
		toCopyDest.Transition.pResource = mBuffersOnGPU.Get();
		toCopyDest.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
		toCopyDest.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		commandList.ResourceBarrier(1, &toCopyDest);

		//コピー
		commandList.CopyResource(mBuffersOnGPU.Get(), uploadBuf.Get());

		//structuredBufferをGENERIC_READへ
		D3D12_RESOURCE_BARRIER toGenericRead = {};
		toGenericRead.Transition.pResource = mBuffersOnGPU.Get();
		toGenericRead.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		toGenericRead.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		commandList.ResourceBarrier(1, &toGenericRead);
		
	}

}

ID3D12Resource* StructuredBuffer::getBufferOnGPU() const
{
	return mBuffersOnGPU.Get();
}

D3D12_RESOURCE_DESC StructuredBuffer::getResourceDesc()
{
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(mSizeOfElement * mNumElement);
	return desc;
}

D3D12_HEAP_PROPERTIES StructuredBuffer::getHeapProperties()
{
	D3D12_HEAP_PROPERTIES prop{};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;	//GPUからの読み取り専用(CPUから書き込めない)

	return prop;
}

void StructuredBuffer::createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop)
{
	//バッファーの確保
	device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mBuffersOnGPU.ReleaseAndGetAddressOf())
	);
}

