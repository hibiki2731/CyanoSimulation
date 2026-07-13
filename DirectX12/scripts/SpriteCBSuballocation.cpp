#include "SpriteCBSuballocation.h"

SpriteCBSuballocation::SpriteCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	:IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{
}

void SpriteCBSuballocation::applyChanges(int frame)
{
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}

void SpriteCBSuballocation::updateWorld(const XMMATRIX& world)
{
	mData.world = world;
}

void SpriteCBSuballocation::updateWindowSize(const XMFLOAT2& windowSize)
{
	mData.windowSize = windowSize;
}

void SpriteCBSuballocation::updateSpriteSize(const XMFLOAT2& spriteSize)
{
	mData.spriteSize = spriteSize;
}

void SpriteCBSuballocation::updateTextureSize(const XMFLOAT2& textureSize)
{
	mData.textureSize = textureSize;
}

void SpriteCBSuballocation::updateBordarSize(const float bordarSize)
{
	mData.bordarSize = bordarSize;
}

