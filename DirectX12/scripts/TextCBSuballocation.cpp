#include "pch.h"
#include "TextCBSuballocation.h"
#include "ConstantBuffer.h"

TextCBSuballocation::TextCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	: IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{

}

void TextCBSuballocation::updateWorld(const XMMATRIX& world)
{
	mData.world = world;
}

void TextCBSuballocation::updateWindowSize(const XMFLOAT2& windowSize)
{
	mData.windowSize = windowSize;
}

void TextCBSuballocation::updateSpriteSize(const XMFLOAT2& spriteSize)
{
	mData.spriteSize = spriteSize;
}

void TextCBSuballocation::updateTextureSize(const XMFLOAT2& textureSize)
{
	mData.textureSize = textureSize;
}

void TextCBSuballocation::updateBordarSize(const float bordarSize)
{
	mData.bordarSize = bordarSize;
}

void TextCBSuballocation::applyChanges(const int frame)
{
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}
