#pragma once
#include "Definition.h"
#include "d3dx12.h"
#include <map>


class AlignedSizeInBytes {
public:
	AlignedSizeInBytes(const int sizeInBytes);
	int get() const { return mSizeInBytes; }

	AlignedSizeInBytes operator=(const AlignedSizeInBytes& other) {
		return AlignedSizeInBytes(other.mSizeInBytes);
	}

	AlignedSizeInBytes operator+(const AlignedSizeInBytes& other) {
		return AlignedSizeInBytes(mSizeInBytes + other.mSizeInBytes);
	}

private:
	int mSizeInBytes;
};

class ConstantBufferAddress {
public:
	ConstantBufferAddress(const int address) : mAddress(address) {
		if (address < 0) {
			assert(false && "ConstantBufferAddressの値は0以上である必要があります。");
		}
	}
	int get() const { return mAddress; }

	auto operator<=>(const ConstantBufferAddress& other) const = default;

	void operator=(const ConstantBufferAddress& other) {
		mAddress = other.mAddress;
	}

private:
	int mAddress;
};

class IConstantBufferSuballocation {
public:
	IConstantBufferSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, const std::vector<void*>&& buffersOnCPU, const std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU);

	const AlignedSizeInBytes& getSizeInBytes() const{
		return mSizeInBytes;
	};

	const ConstantBufferAddress& getBufferAddress() const {
		return mBufferAddress;
	};

	const void* getVirtualAddressOnCPU(const int frame) const;
	const D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddressOnGPU(const int frame) const;

	virtual void applyChanges(const int frame) = 0;

protected:
	const AlignedSizeInBytes mSizeInBytes;
	const ConstantBufferAddress mBufferAddress;
	const std::vector<void*> mBuffersOnCPU;
	const std::vector<D3D12_GPU_VIRTUAL_ADDRESS>  mVirtualAddressOnGPU;
};

class ConstantBufferSuballocator {
public:
	ConstantBufferSuballocator(const AlignedSizeInBytes& sizeInBytes, const std::vector<void*>& buffersOnCPU, const std::vector<ComPtr<ID3D12Resource>>& buffersOnGPU);

	//スロットを確保する
	template<typename T>
	std::shared_ptr<T> allocate(const AlignedSizeInBytes& requiredSize)
	{
		if (requiredSize.get() + mLastUsedAddress.get() >= mMaxSizeInBytes.get()) {
			assert(false && "コンスタントバッファの容量が不足しています。");
		}

		//解放されたヒープがあれば優先して使う
		for (const auto& [emptySuballocationAddress, emptySuballocationSize] : mClearedSuballocation) {
			//要求サイズより、空いているサイズが大きければ使用する
			if (emptySuballocationSize.get() >= requiredSize.get()) {
				AlignedSizeInBytes remainedEmptySuballocationSize(emptySuballocationSize.get() - requiredSize.get());			//要求サイズ分空いているヒープを減らす
				ConstantBufferAddress newSuballocationAddress = emptySuballocationAddress;

				//空いているスロットが0より大きい場合
				if (remainedEmptySuballocationSize.get() > 0)
				{
					ConstantBufferAddress remainedEmptySuballocationAddress(emptySuballocationAddress.get() + requiredSize.get());
					mClearedSuballocation.insert(std::map<ConstantBufferAddress, AlignedSizeInBytes>::value_type(remainedEmptySuballocationAddress, remainedEmptySuballocationSize));
				}
				mClearedSuballocation.erase(emptySuballocationAddress);

				auto buffersOnCPU = getSuballocationAddressOnCPU(newSuballocationAddress);
				auto virtualAddressOnGPU = getSuballocationAddressOnGPU(newSuballocationAddress);

				auto suballocation = std::make_shared<T>(requiredSize, newSuballocationAddress, std::move(buffersOnCPU), std::move(virtualAddressOnGPU));
				return suballocation;
			}
		}

		//解放されたヒープがなければ、最後尾のインデックスを取得
		ConstantBufferAddress newSuballocationAddress = mLastUsedAddress;
		mLastUsedAddress = ConstantBufferAddress(mLastUsedAddress.get() + requiredSize.get());
		auto buffersOnCPU = getSuballocationAddressOnCPU(newSuballocationAddress);
		auto virtualAddressOnGPU = getSuballocationAddressOnGPU(newSuballocationAddress);

		auto suballocation = std::make_shared<T>(requiredSize, newSuballocationAddress, std::move(buffersOnCPU), std::move(virtualAddressOnGPU));
		return suballocation;
	}
	void free(const IConstantBufferSuballocation& subaalocation);

private:
	std::vector<void*> getSuballocationAddressOnCPU(const ConstantBufferAddress& address);
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> getSuballocationAddressOnGPU(const ConstantBufferAddress& address);

	const AlignedSizeInBytes mMaxSizeInBytes;
	ConstantBufferAddress mLastUsedAddress;
	std::map<ConstantBufferAddress, AlignedSizeInBytes> mClearedSuballocation;
	const std::vector<void*>& mBuffersOnCPU;
	const std::vector<ComPtr<ID3D12Resource>>& mBuffersOnGPU;
};

class ConstantBuffer
{
public:
	ConstantBuffer(class Graphic& graphic, UINT sizeInBytes);
	~ConstantBuffer();

	//サブアロケーションを追加
	template<typename T> 
	std::shared_ptr<T> createSuballocation(const AlignedSizeInBytes& sizeInBytes)
	{
		return mSuballocator->allocate<T>(sizeInBytes);
	}

	void deleteSuballocation(const IConstantBufferSuballocation& suballocation);

private:
	D3D12_HEAP_PROPERTIES getHeapProperties(); //ヒープのプロパティ
	D3D12_RESOURCE_DESC getResourceDesc(int sizeInBytes); //リソースの詳細設定
	void createAndMapBuffers(ID3D12Device& device, D3D12_HEAP_PROPERTIES& prop, D3D12_RESOURCE_DESC& desc); //バッファの作成とマッピング

	std::unique_ptr<ConstantBufferSuballocator> mSuballocator;
	std::vector<ComPtr<ID3D12Resource>> mBuffersOnGPU; //コンスタントバッファのリソース
	std::vector<void*> mBuffersOnCPU; //コンスタントバッファの生データのアドレス
};

