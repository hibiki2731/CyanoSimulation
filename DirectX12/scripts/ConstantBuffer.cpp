#include "pch.h"
#include "ConstantBuffer.h"
#include "Graphic.h"

ConstantBuffer::ConstantBuffer(Graphic& graphic, UINT sizeInBytes)
{
	assert(sizeInBytes > 0 && "ConstantBufferのサイズは1以上である必要があります。");

	auto device = graphic.getDevice();

	auto prop = getHeapProperties();
	auto desc = getResourceDesc(sizeInBytes);

	//バッファの確保とCPU上へのマップ
	createAndMapBuffers(*device, prop, desc);

	//サブアロケータの作成
	mSuballocator = std::make_unique<ConstantBufferSuballocator>(sizeInBytes, mBuffersOnCPU, mBuffersOnGPU);

}

ConstantBuffer::~ConstantBuffer()
{
	for(int i = 0; i < mBuffersOnGPU.size(); i++) {
		mBuffersOnGPU[i]->Unmap(0, nullptr);
	}
}

void ConstantBuffer::deleteSuballocation(const IConstantBufferSuballocation& suballocation)
{
	mSuballocator->free(suballocation);
}

D3D12_HEAP_PROPERTIES ConstantBuffer::getHeapProperties()
{
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD; //CPUからGPUへ書き込み
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	return prop;
}

D3D12_RESOURCE_DESC ConstantBuffer::getResourceDesc(int sizeInBytes)
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

void ConstantBuffer::createAndMapBuffers(ID3D12Device& device, D3D12_HEAP_PROPERTIES& prop, D3D12_RESOURCE_DESC& desc)
{
	int frameCount = Graphic::FrameCount;
	mBuffersOnGPU.resize(frameCount);
	mBuffersOnCPU.resize(frameCount);

	for (int i = 0; i < frameCount; i++) {
		HRESULT hr = device.CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(mBuffersOnGPU[i].ReleaseAndGetAddressOf())
		);
		assert(SUCCEEDED(hr));

		//GPUに確保したバッファをCPUからアクセス可能なアドレス空間にマッピングする
		CD3DX12_RANGE readRange(0, 0); //CPUからGPUへの書き込みのみなので、読み取り範囲は0
		hr = mBuffersOnGPU[i]->Map(0, &readRange, reinterpret_cast<void**>(&mBuffersOnCPU[i]));
		assert(SUCCEEDED(hr));
	}
}

AlignedSizeInBytes::AlignedSizeInBytes(const int sizeInBytes)
	:mSizeInBytes((sizeInBytes + 0xff) & ~0xff)
{
}

IConstantBufferSuballocation::IConstantBufferSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, const std::vector<void*>&& buffersOnCPU, const std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	:mSizeInBytes(sizeInBytes),
	mBufferAddress(address), 
	mBuffersOnCPU(buffersOnCPU),
	mVirtualAddressOnGPU(virtualAddressOnGPU)
{
}

const void* IConstantBufferSuballocation::getVirtualAddressOnCPU(const int frame) const
{
	return mBuffersOnCPU[frame];
}

const D3D12_GPU_VIRTUAL_ADDRESS IConstantBufferSuballocation::getVirtualAddressOnGPU(const int frame) const
{
	return mVirtualAddressOnGPU[frame];
}

ConstantBufferSuballocator::ConstantBufferSuballocator(const AlignedSizeInBytes& sizeInBytes, const std::vector<void*>& buffersOnCPU, const std::vector<ComPtr<ID3D12Resource>>& buffersOnGPU)
	:mMaxSizeInBytes(sizeInBytes),
	mLastUsedAddress(0),
	mBuffersOnCPU(buffersOnCPU),
	mBuffersOnGPU(buffersOnGPU)
{
}


void ConstantBufferSuballocator::free(const IConstantBufferSuballocation& subaalocation)
{
	ConstantBufferAddress deleteAddress = subaalocation.getBufferAddress();
	AlignedSizeInBytes deleteSize = subaalocation.getSizeInBytes();
	auto rightSuballocationIter = mClearedSuballocation.lower_bound(deleteAddress.get());
	auto leftSuballocationIter = rightSuballocationIter; //左側の解放済みスロットを参照するためのイテレータ

	//解放するヒープの直前に連続する解放されたヒープがあるか確認
	if (leftSuballocationIter != mClearedSuballocation.begin()) {
		leftSuballocationIter--;
		const ConstantBufferAddress leftSuballocationAddress(leftSuballocationIter->first.get() + leftSuballocationIter->second.get());

		//連続していた場合
		if (leftSuballocationAddress == deleteAddress) {
			deleteAddress = leftSuballocationIter->first;
			deleteSize = deleteSize + leftSuballocationIter->second;
		}
	}

	//解放するヒープの直後に連続する解放されたヒープがあるか確認
	if (rightSuballocationIter != mClearedSuballocation.end()) {
		if (rightSuballocationIter->first.get() == deleteAddress.get() + deleteSize.get()) {
			//連続していた場合
			deleteSize = deleteSize + rightSuballocationIter->second;
			mClearedSuballocation.erase(rightSuballocationIter);
		}
	}

	//解放するヒープを解放済みヒープに追加
	mClearedSuballocation.insert(std::map<ConstantBufferAddress, AlignedSizeInBytes>::value_type(std::move(deleteAddress), std::move(deleteSize)));
	
}

std::vector<void*> ConstantBufferSuballocator::getSuballocationAddressOnCPU(const ConstantBufferAddress& address)
{
	std::vector<void*> buffersOnCPU(mBuffersOnCPU.size());
	for (int i = 0; i < mBuffersOnCPU.size(); i++) {
		buffersOnCPU[i] = static_cast<uint8_t*>(mBuffersOnCPU[i]) + address.get();
	}

	return buffersOnCPU;
}

std::vector<D3D12_GPU_VIRTUAL_ADDRESS> ConstantBufferSuballocator::getSuballocationAddressOnGPU(const ConstantBufferAddress& address)
{
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> virtualAddressOnGPU(mBuffersOnGPU.size());
	for (int i = 0; i < mBuffersOnGPU.size(); i++) {
		virtualAddressOnGPU[i] = mBuffersOnGPU[i]->GetGPUVirtualAddress() + address.get();
	}

	return virtualAddressOnGPU;
}
