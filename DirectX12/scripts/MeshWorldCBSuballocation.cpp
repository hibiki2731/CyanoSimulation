#include "MeshWorldCBSuballocation.h"

MeshWorldCBSuballocation::MeshWorldCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	: IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{
}

void MeshWorldCBSuballocation::updateWorld(const XMMATRIX& world)
{
	mData.world = world;
}

void MeshWorldCBSuballocation::updateFlashColor(const XMFLOAT3& flashColor)
{
	mData.flashColor = flashColor;
}

void MeshWorldCBSuballocation::updateFlashIntensity(float flashIntensity)
{
	mData.flashIntensity = flashIntensity;
}

void MeshWorldCBSuballocation::applyChanges(const int frame)
{
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}
