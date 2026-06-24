#pragma once
class NumSlots {
public:
	NumSlots(const int numSlots) : mNumSlots(numSlots) {
		if (numSlots < 0) {
			assert(false && "NumSlotsの値は0以上である必要があります。");
		}
	}
	int getNumSlots() const { return mNumSlots; }

	bool operator>(const NumSlots& other) const {
		return mNumSlots > other.mNumSlots;
	}

	bool operator>=(const NumSlots& other) const {
		return mNumSlots >= other.mNumSlots;
	}

	bool operator==(const NumSlots& other) const {
		return mNumSlots == other.mNumSlots;
	}

	NumSlots operator-(const NumSlots& other) const {
		return NumSlots(mNumSlots - other.mNumSlots);
	}

	NumSlots operator=(const NumSlots& other) {
		return NumSlots(other.mNumSlots);
	}

private:
	const int mNumSlots;
};

class SlotIndex {
public:
	SlotIndex(const int index) : mIndex(index) {
		if (index < 0) {
			assert(false && "SlotIndexの値は0以上である必要があります。");
		}
	}
	int getIndex() const { return mIndex; }

	SlotIndex operator=(const SlotIndex& other) {
		return SlotIndex(other.mIndex);
	}

private:
	const int mIndex;
};

// 0スロットを表すNumSlotsの定数
extern const NumSlots ZeroSlot;

class DescriptorAllocatorRange {
public:
	DescriptorAllocatorRange(const SlotIndex& startIndex, const NumSlots& numSlots)
		: mStartIndex(startIndex), mNumSlots(numSlots) {
	}

	const SlotIndex getIndex(int offset = 0) const {
		if (offset < 0 || offset >= mNumSlots.getNumSlots()) {
			assert(false && "DescriptorAllocatorRangeのオフセットは範囲内である必要があります。");
		}

		return SlotIndex(mStartIndex.getIndex() + offset);
	}

private:
	const SlotIndex mStartIndex;
	const NumSlots mNumSlots;
};


class DescriptorHeapAllocator
{
public:
	DescriptorHeapAllocator(const NumSlots& numSlots);

	//スロットを確保する
	DescriptorAllocatorRange allocateRange(const NumSlots& numRequiredSlots);
	DescriptorAllocatorRange allocate();
	void freeSlot(const DescriptorAllocatorRange& allocRange);

private:
	const NumSlots mNumAllSlots;
	SlotIndex mLastUsedSlotIndex;
	std::map<SlotIndex, NumSlots> mClearedHeap;
};

