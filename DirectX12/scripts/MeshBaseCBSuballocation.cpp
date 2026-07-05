#include "MeshBaseCBSuballocation.h"

MeshBaseCBSuballocation::MeshBaseCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU)
	: IConstantBufferSuballocation(sizeInBytes, address, std::move(buffersOnCPU), std::move(virtualAddressOnGPU))
{
}

void MeshBaseCBSuballocation::updateViewProj(const XMMATRIX& viewProj)
{
	mData.viewProj = viewProj;
}

void MeshBaseCBSuballocation::updateCameraPos(const XMFLOAT4& cameraPos)
{
	mData.cameraPos = cameraPos;
}

void MeshBaseCBSuballocation::updatePointLights(const PointLightData& light)
{
	if(mCurrentPointLightNum >= MAX_LIGHT_NUM) return;
	

	mData.pointLights[mCurrentPointLightNum].position   = light.position;
	mData.pointLights[mCurrentPointLightNum].color      = light.color;
	mData.pointLights[mCurrentPointLightNum].setValue = light.setValue;

	mCurrentPointLightNum++;
}

void MeshBaseCBSuballocation::updateSpotLights(const SpotLightData& light)
{
	if (mCurrentSpotLightNum >= MAX_LIGHT_NUM) return;

	mData.spotLights[mCurrentSpotLightNum].position   = light.position;
	mData.spotLights[mCurrentSpotLightNum].direction  = light.direction;
	mData.spotLights[mCurrentSpotLightNum].color      = light.color;
	mData.spotLights[mCurrentSpotLightNum].setValue = light.setValue;
	mData.spotLights[mCurrentSpotLightNum].attAngle = light.attAngle;

	mCurrentSpotLightNum++;
}

void MeshBaseCBSuballocation::updatePlayerFlashIntensity(float intensity)
{
	mData.playerFlashIntensity = intensity;
}

void MeshBaseCBSuballocation::updatePlayerFlashColor(const XMFLOAT3& color)
{
	mData.playerFlashColor = color;
}

void MeshBaseCBSuballocation::applyChanges(const int frame)
{
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}

void MeshBaseCBSuballocation::applyPointLights(const int frame)
{
	//ライトの数が減った場合、減った分のライトを無効にする
	if (mPrePointLightNum > mCurrentPointLightNum) {
		for (int i = mCurrentPointLightNum; i < mPrePointLightNum; i++) {
			mData.pointLights[i].setValue.x = 0;	//ライトを無効にする
		}
	}

	//更新したデータをコンスタントバッファへコピー
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());


	//前フレームのライトの数を保存
	mPrePointLightNum = mCurrentPointLightNum;

	//インデックスをリセット
	mCurrentPointLightNum = 0;
}

void MeshBaseCBSuballocation::applySpotLights(const int frame)
{
	if (mPreSpotLightNum > mCurrentSpotLightNum) {
		for (int i = mCurrentSpotLightNum; i < mPreSpotLightNum; i++) {
			mData.spotLights[i].setValue.x = 0;	//ライトを無効にする
		}
	}
	//前フレームのライトの数を保存
	mPreSpotLightNum = mCurrentSpotLightNum;
	//インデックスをリセット
	mCurrentSpotLightNum = 0;
	//更新したデータをコンスタントバッファへコピー
	memcpy(mBuffersOnCPU[frame], &mData, mSizeInBytes.get());
}
