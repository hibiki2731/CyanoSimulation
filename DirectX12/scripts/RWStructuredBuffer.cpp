#include "pch.h"
#include "RWStructuredBuffer.h"
#include "Graphic.h"

RWStructuredBuffer::RWStructuredBuffer(ID3D12Device& device, int sizeOfElement, int numElement)
	:mSizeOfElement(sizeOfElement)
	, mNumElement(numElement)
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

RWStructuredBuffer::~RWStructuredBuffer()
{
	//アンマップ
	CD3DX12_RANGE range(0, 0);
	mBuffersOnGPU->Unmap(0, &range);
}

void RWStructuredBuffer::copyData(const void* resource, const size_t resourceSize)
{
	memcpy(mBuffersOnCPU, resource, resourceSize);
}

ID3D12Resource* RWStructuredBuffer::getBufferOnGPU() const
{
	return mBuffersOnGPU.Get();
}

void* RWStructuredBuffer::getBufferOnCPU() const
{
	return mBuffersOnCPU;
}

D3D12_RESOURCE_DESC RWStructuredBuffer::getResourceDesc()
{
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(mSizeOfElement * mNumElement);
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;	//アンオーダーアクセスに設定
	return desc;
}

D3D12_HEAP_PROPERTIES RWStructuredBuffer::getHeapProperties()
{
	D3D12_HEAP_PROPERTIES prop{};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;		//ヒープ設定を手動で行う
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	//CPUがキャッシュへ書き込む際、ライトバック方式をとるよう設定
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;			//メモリの配置先をL0プールに指定
	prop.CreationNodeMask = 1;									//デフォルトのプライマリGPUでメモリを確保
	prop.VisibleNodeMask = 1;									//デフォルトのプライマリGPUからメモリが見えるようにする

	return prop;
}

void RWStructuredBuffer::createAndMapBuffers(ID3D12Device& device, D3D12_RESOURCE_DESC& desc, D3D12_HEAP_PROPERTIES& prop)
{
	int bufferNo = 0;
	//バッファーの確保
	device.CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(mBuffersOnGPU.ReleaseAndGetAddressOf())
	);

	//GPUに確保したバッファをCPUからアクセス可能なアドレス空間にマッピングする
	CD3DX12_RANGE readRange(0, 0); //CPUからGPUへの書き込みのみなので、読み取り範囲は0
	mBuffersOnGPU->Map(0, &readRange, reinterpret_cast<void**>(&mBuffersOnCPU));

}

