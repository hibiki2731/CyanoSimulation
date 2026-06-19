#include "pch.h"
#include "Graphic.h"
#include "UnorderedAccessBuffer.h"

UnorderedAccessBuffer::UnorderedAccessBuffer(Graphic& graphic, int sizeOfElement, int numElement)
	:mSizeOfElement(sizeOfElement)
	, mNumElement(numElement)
{
	auto device = graphic.getDevice();
	//バッファーの詳細設定
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(mSizeOfElement * mNumElement);
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;	//アンオーダーアクセスに設定
	int bufferNo = 0;

	D3D12_HEAP_PROPERTIES prop{};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;		//ヒープ設定を手動で行う
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	//CPUがキャッシュへ書き込む際、ライトバック方式をとるよう設定
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;			//メモリの配置先をL0プールに指定
	prop.CreationNodeMask = 1;									//デフォルトのプライマリGPUでメモリを確保
	prop.VisibleNodeMask = 1;									//デフォルトのプライマリGPUからメモリが見えるようにする

	mBuffersOnGPU.resize(Graphic::FrameCount);
	mBuffersOnCPU.resize(Graphic::FrameCount);
	//バッファーの確保
	for (auto& buffer : mBuffersOnGPU) {
		device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
		);

		//GPUに確保したバッファをCPUからアクセス可能なアドレス空間にマッピングする
		CD3DX12_RANGE readRange(0, 0); //CPUからGPUへの書き込みのみなので、読み取り範囲は0
		buffer->Map(0, &readRange, reinterpret_cast<void**>(&mBuffersOnCPU[bufferNo]));

		bufferNo++;
	}
	
}

void UnorderedAccessBuffer::init()
{

}
