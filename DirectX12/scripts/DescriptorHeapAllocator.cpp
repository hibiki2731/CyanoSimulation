#include "pch.h"
#include "DescriptorHeapAllocator.h"

const NumSlots ZeroSlot(0);

DescriptorHeapAllocator::DescriptorHeapAllocator(const NumSlots& numSlots)
	: mNumAllSlots(numSlots), mLastUsedSlotIndex(0)
{
}

DescriptorAllocatorRange DescriptorHeapAllocator::allocateRange(const NumSlots& numRequiredSlots)
{
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
}
