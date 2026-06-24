#include "pch.h"
#include "Graphic.h"
#include "DescriptorHeap.h"
#include "UnorderedAccessBuffer.h"

const NumSlots ZeroSlot(0);

DescriptorHeapAllocator::DescriptorHeapAllocator(const NumSlots& numSlots)
	: mNumAllSlots(numSlots), mLastUsedSlotIndex(0)
{
}

DescriptorAllocatorRange DescriptorHeapAllocator::allocateRange(const NumSlots& numRequiredSlots)
{
	if (numRequiredSlots.getNumSlots() + mLastUsedSlotIndex.getIndex() >= mNumAllSlots.getNumSlots()) {
		assert(false && "DescriptorHeapAllocatorのスロットが不足しています。");
	}

	//解放されたヒープがあれば優先して使う
	for (auto& [EmptySlotIndex, numEmptySlots] : mClearedHeap) {
		//要求サイズより、空いているサイズが大きければ使用する
		if (numEmptySlots >= numRequiredSlots) {
			const NumSlots numRemainedEmptySlots = numEmptySlots - numRequiredSlots;			//要求サイズ分空いているヒープを減らす
			const SlotIndex newSlotIndex = EmptySlotIndex;

			//空いているスロットが0より大きい場合
			if (numRemainedEmptySlots > ZeroSlot)
			{
				const SlotIndex remainedEmptySlotIndex(EmptySlotIndex.getIndex() + numRequiredSlots.getNumSlots());
				mClearedHeap[remainedEmptySlotIndex] = numRemainedEmptySlots;
			}
			mClearedHeap.erase(EmptySlotIndex);

			return DescriptorAllocatorRange(newSlotIndex, numRequiredSlots);
		}
	}

	//解放されたヒープがなければ、最後尾のインデックスを取得
	const SlotIndex newSlotIndex = mLastUsedSlotIndex;
	mLastUsedSlotIndex = SlotIndex(newSlotIndex.getIndex() + numRequiredSlots.getNumSlots());
	return DescriptorAllocatorRange(newSlotIndex, numRequiredSlots);
}

DescriptorAllocatorRange DescriptorHeapAllocator::allocate()
{
	const NumSlots numRequiredSlots(1);
	return allocateRange(numRequiredSlots);
}

void DescriptorHeapAllocator::freeSlot(const DescriptorAllocatorRange& allocRange)
{
	SlotIndex initialIndex = allocRange.getIndex(0);
	NumSlots numFullSlots = allocRange.getNumSlots();
	auto rightClearedSlotIter = mClearedHeap.lower_bound(allocRange.getIndex(0));
	auto leftClearedSlotIter = rightClearedSlotIter; //左側の解放済みスロットを参照するためのイテレータ

	//解放するヒープの直前に連続する解放されたヒープがあるか確認
	if (leftClearedSlotIter != mClearedHeap.begin()) {
		leftClearedSlotIter--;
		const SlotIndex leftClearedSlotLastIndex = SlotIndex(leftClearedSlotIter->first.getIndex() + leftClearedSlotIter->second.getNumSlots());

		//連続していた場合
		if (leftClearedSlotLastIndex == allocRange.getIndex(0)) {
			initialIndex = leftClearedSlotIter->first;
			numFullSlots = numFullSlots + leftClearedSlotIter->second;
		}
	}

	//解放するヒープの直後に連続する解放されたヒープがあるか確認
	if (rightClearedSlotIter != mClearedHeap.end()) {
		if (rightClearedSlotIter->first.getIndex() == initialIndex.getIndex() + numFullSlots.getNumSlots()) {
			//連続していた場合
			numFullSlots = numFullSlots + rightClearedSlotIter->second;
			mClearedHeap.erase(rightClearedSlotIter);
		}
	}

	mClearedHeap[initialIndex] = numFullSlots;
	
}

DescriptorHeap::DescriptorHeap(Graphic& graphic, const NumSlots& numSlots)
	: mGraphic(graphic)
{

	//ディスクリプタヒープの詳細設定
	auto desc = getHeapDesc(numSlots);

	//ディスクリプタヒープの作成
	auto device = graphic.getDevice();
	createHeap(*device, desc);

	//アロケータの作成
	mHeapAllocator = std::make_unique<DescriptorHeapAllocator>(numSlots);
}

DescriptorAllocatorRange DescriptorHeap::allocate(const NumSlots& numRequiredSlots)
{
	return mHeapAllocator->allocateRange(numRequiredSlots);
}

void DescriptorHeap::addUAV(UnorderedAccessBuffer& uav, const SlotIndex& slotIndex)
{
	auto device = mGraphic.getDevice();

	//UAVのディスクリプタを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.NumElements = static_cast<UINT>(uav.getNumElements());
	desc.Buffer.StructureByteStride = static_cast<UINT>(uav.getSizeOfElement());

	//ディスクリプタヒープのCPUハンドルを取得し、空きスロットにUAVを作成
	auto cpuHandle = getCPUHandle(slotIndex);

	//UAVを作成
	device->CreateUnorderedAccessView(
		uav.getBufferOnGPU(),
		nullptr,
		&desc,
		cpuHandle
	);
}

D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeap::getHeapDesc(const NumSlots& numSlots)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = numSlots.getNumSlots();
	desc.NodeMask = 0;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	return desc;
}

void DescriptorHeap::createHeap(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	HRESULT hr = device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDescHeap.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getCPUHandle(const SlotIndex& slotIndex)
{
	auto cpuHandle = mDescHeap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += slotIndex.getIndex() * mGraphic.getCbvTbvIncSize();
	return cpuHandle;
}

