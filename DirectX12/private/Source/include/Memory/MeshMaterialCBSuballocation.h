#pragma once
#include "ConstantBuffer.h"

//メッシュごとのマテリアル情報
struct MeshMaterialCBSuballocationData {
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;
};

class MeshMaterialCBSuballocation : public IConstantBufferSuballocation
{
public:
	MeshMaterialCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU);

	void updateAmbient(const XMFLOAT4& ambient);
	void updateDiffuse(const XMFLOAT4& diffuse);
	void updateSpecular(const XMFLOAT4& specular);
	
	void applyChanges(const int frame) override;
	void applyChanges();

private:
	MeshMaterialCBSuballocationData mData;
};

