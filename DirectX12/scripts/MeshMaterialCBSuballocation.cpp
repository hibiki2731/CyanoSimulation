#include "MeshMaterialCBSuballocation.h"

MeshMaterialCBSuballocation::MeshMaterialCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	: IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{
}

void MeshMaterialCBSuballocation::updateAmbient(const XMFLOAT4& ambient)
{
	mData.ambient = ambient;
}

void MeshMaterialCBSuballocation::updateDiffuse(const XMFLOAT4& diffuse)
{
	mData.diffuse = diffuse;
}

void MeshMaterialCBSuballocation::updateSpecular(const XMFLOAT4& specular)
{
	mData.specular = specular;
}

void MeshMaterialCBSuballocation::applyChanges(const int frame)
{
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}

void MeshMaterialCBSuballocation::applyChanges()
{
	memcpy(mBuffersOnCPU[0], &mData, mSizeInBytes.get());
	memcpy(mBuffersOnCPU[1], &mData, mSizeInBytes.get());
}
