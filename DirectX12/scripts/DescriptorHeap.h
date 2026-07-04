#pragma once
#include "Definition.h"
#include "d3dx12.h"
#include <map>
class UnorderedAccessBuffer;


//ディスクリプタヒープのスロット数を表すクラス
class NumSlots {
public:
	NumSlots(const int numSlots) : mNumSlots(numSlots) {
		if (numSlots < 0) {
			assert(false && "NumSlotsの値は0以上である必要があります。");
		}
	}
	int getNumSlots() const { return mNumSlots; }

	auto operator<=>(const NumSlots& other) const = default;

	NumSlots operator-(const NumSlots& other) const {
		return NumSlots(mNumSlots - other.mNumSlots);
	}

	NumSlots operator+(const NumSlots& other) const {
		return NumSlots(mNumSlots + other.mNumSlots);
	}

	NumSlots operator=(const NumSlots& other) {
		return NumSlots(other.mNumSlots);
	}

private:
	int mNumSlots;
};

// 0スロットを表すNumSlotsの定数
extern const NumSlots ZeroSlot;

//ディスクリプタヒープのスロットインデックスを表すクラス
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

	SlotIndex operator+(const SlotIndex& other) const {
		return SlotIndex(mIndex + other.getIndex());
	}

	auto operator<=>(const SlotIndex& other) const = default;

private:
	int mIndex;
};

// ディスクリプタヒープのスロット範囲を表すクラス
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
	
	const NumSlots getNumSlots() const {
		return mNumSlots;
	}

private:
	const SlotIndex mStartIndex;
	const NumSlots mNumSlots;
};

// ディスクリプタヒープのスロットを管理するアロケータ
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

class DescriptorHeap
{
public:
	DescriptorHeap(class Graphic& graphic, const NumSlots& numSlots);


	//ビューを追加する前に、必要なスロット数を確保する
	DescriptorAllocatorRange allocate(const NumSlots& numRequiredSlots);

	//ビューを追加する
	void addUAV(UnorderedAccessBuffer& uav, const SlotIndex& slotIndex);
	void addSRV(ID3D12Resource& shaderResource, const SlotIndex& slotIndex);
	void addCBV(class IConstantBufferSuballocation& cbv, const SlotIndex& slotIndex, const int frame);

	//アドレスの取得
	ID3D12DescriptorHeap* const* getAddress() const { return mDescHeap.GetAddressOf(); }
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(const SlotIndex& slotIndex);

private:
	D3D12_DESCRIPTOR_HEAP_DESC getHeapDesc(const NumSlots& numSlots);
	void createHeap(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_DESC& desc);
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(const SlotIndex& slotIndex);


	ComPtr<ID3D12DescriptorHeap> mDescHeap;

	//ディスクリプタヒープのスロットを管理するアロケータ
	std::unique_ptr<DescriptorHeapAllocator> mHeapAllocator;

	class Graphic& mGraphic;
};

