#include "pch.h"
#include "TextCBSuballocation.h"
#include "ConstantBuffer.h"

TextCBSuballocation::TextCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	: IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{

}

void TextCBSuballocation::updateWorld(const XMMATRIX& world, const int frame)
{

	mData.world = world;
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}

void TextCBSuballocation::updateData()
{
	memcpy(mBuffersOnCPU[0], &mData, mSizeInBytes.get());
	memcpy(mBuffersOnCPU[1], &mData, mSizeInBytes.get());
}
