#pragma once
#include "ConstantBuffer.h"

//3Dオブジェクトのワールド行列
struct MeshWorldCBSuballocationData {
	XMMATRIX world;  //ワールド行列
	XMFLOAT3 flashColor;
	float flashIntensity;
};

class MeshWorldCBSuballocation : public IConstantBufferSuballocation
{
public:
	MeshWorldCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU);

	void updateWorld(const XMMATRIX& world);
	void updateFlashColor(const XMFLOAT3& flashColor);
	void updateFlashIntensity(float flashIntensity);

	void applyChanges(const int frame) override;

private:
	MeshWorldCBSuballocationData mData;
};

