#include "CameraComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Scene.h"
#include "Math.h"
#include "DamageText.h"
#include "DungeonScene.h"
#include "PerlinNoise1D.h"
#include "Random.h"
#include <windows.h>

float CameraComponent::AccumulatedTime = 0.0f;

CameraComponent::CameraComponent(Actor& owner, DungeonScene& scene, int updateOrder)
	: Component(owner, updateOrder),
	mDungeonScene(scene)
{
	mFront = { 0, 0, 1.0f };  mUp = { 0, 1, 0 };
	mFocus = mOwner.getPosition() + mFront;
	mRot = 0;
	isActive = false;
	mPerlinNoise = std::make_unique<PerlinNoise1D>(Random::dist(0, 10000));
}

void CameraComponent::inputComponent()
{
}

void CameraComponent::updateComponent()
{
	if (isActive) {
		mFront = { 0.0f, 0.0f, 1.0f };
		mFront = Math::rotateX(mFront, mOwner.getRotation().x);
		mFront = Math::rotateY(mFront, mOwner.getRotation().y);
		mFront = Math::rotateZ(mFront, mOwner.getRotation().z);
		mFocus = mOwner.getPosition() + mFront;
		XMFLOAT3 eye = mOwner.getPosition() + applyShake();


		XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&mFocus), XMLoadFloat3(&mUp));
		//プロジェクションマトリックス
		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mOwner.getScene().getGame().getGraphic().getAspect(), 0.01f, 50.0f);
		XMMATRIX viewProj = view * proj;
		mDungeonScene.updateDTView(view);
		mOwner.getScene().getGame().getGraphic().updateViewProj(viewProj);

		XMFLOAT4 cameraPos;
		cameraPos.x = eye.x;
		cameraPos.y = eye.y;
		cameraPos.z = eye.z;
		cameraPos.w = 1;
		mOwner.getScene().getGame().getGraphic().updateCameraPos(cameraPos);
	}

}

void CameraComponent::setActive(bool state)
{
	isActive = state;
}

void CameraComponent::startShake()
{
	mShake.time = mShake.duration;
}

const float& CameraComponent::getRot()
{
	return mRot;
}

const XMFLOAT3& CameraComponent::getFront()
{
	return mFront;
}

void CameraComponent::setRot(float rot)
{
	mRot = rot;
}

XMFLOAT3 CameraComponent::applyShake()
{
	if (mShake.time <= 0.0f) return XMFLOAT3(0, 0, 0);

	mShake.time -= deltaTime;
	AccumulatedTime += deltaTime;

	//揺れの強さを時間経過に応じて減衰させる
	float t = mShake.time / mShake.duration;
	float strength = mShake.intensity * t * t;

	//ランダムな方向に揺らす
	float offsetsX = mPerlinNoise->noise(mShake.frequency * AccumulatedTime) * strength;
	float offsetsY = mPerlinNoise->noise(mShake.frequency * (AccumulatedTime + 100.0f)) * strength;
	float offsetsZ = mPerlinNoise->noise(mShake.frequency * (AccumulatedTime + 200.0f)) * strength;

	return XMFLOAT3(offsetsX, offsetsY, offsetsZ);
}
