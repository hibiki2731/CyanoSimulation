#pragma once
#include "ConstantBuffer.h"

struct MeshBaseCBSuballocationData {
	XMMATRIX viewProj;  //ビュー射影行列
	XMFLOAT4 cameraPos; //xyz:カメラ位置、w:未使用
	PointLightData pointLights[MAX_LIGHT_NUM];  //ポイントライト配列
	SpotLightData spotLights[MAX_LIGHT_NUM];  //スポットライト配列
	float playerFlashIntensity; //ダメージを受けたときの点滅フラグ
	XMFLOAT3 playerFlashColor;
};

class MeshBaseCBSuballocation : public IConstantBufferSuballocation
{
public:
	MeshBaseCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU);

	void updateViewProj(const XMMATRIX& viewProj);
	void updateCameraPos(const XMFLOAT4& cameraPos);
	void updatePointLights(const PointLightData& pointLight);
	void updateSpotLights(const SpotLightData& spotLight);
	void updatePlayerFlashIntensity(float intensity);
	void updatePlayerFlashColor(const XMFLOAT3& color);

	void applyChanges(const int frame);

private:
	void applyPointLights(const int frame);
	void applySpotLights(const int frame);
	MeshBaseCBSuballocationData mData;

	int mPrePointLightNum = 0;
	int mPreSpotLightNum = 0;
	int mCurrentPointLightNum = 0;
	int mCurrentSpotLightNum = 0;

};

