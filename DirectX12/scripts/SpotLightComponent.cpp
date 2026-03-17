#include "SpotLightComponent.h"
#include "Actor.h"
#include "Math.h"
#include "Game.h"

SpotLightComponent::SpotLightComponent(Actor* owner, int updateOrder) : Component(owner, updateOrder)
{
	isActive = false;
	mPosition.w = 1.0f;
	mIntensity = 1.0f;
	mRange = 1.0f;
	mColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mUAngle = 0.0f;
	mPAngle = 0.0f;
	mDirection = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	mOwner->getGame()->addSpotLight(this);
}

void SpotLightComponent::inputComponent()
{
}

void SpotLightComponent::updateComponent()
{
	if (isActive) {
		mPosition.x = mOwner->getPosition().x;
		mPosition.y = mOwner->getPosition().y;
		mPosition.z = mOwner->getPosition().z;
		// ライトの向きをオーナーの前方ベクトルに設定
		XMFLOAT3 forward = { 0.0f, 0.0f, 1.0f }; // オーナーの前方ベクトル
		forward = Math::rotateX(forward, mOwner->getRotation().x);
		forward = Math::rotateY(forward, mOwner->getRotation().y);
		forward = Math::rotateZ(forward, mOwner->getRotation().z);
		mDirection.x = forward.x;
		mDirection.y = forward.y;
		mDirection.z = forward.z;
	}
}

void SpotLightComponent::endProccess()
{
	//Gameからライトを削除
	mOwner->getGame()->removeSpotLight(this);
}

XMFLOAT4 SpotLightComponent::getPosition()
{
	return mPosition;
}

XMFLOAT4 SpotLightComponent::getDirection()
{
	return mDirection;
}

XMFLOAT4 SpotLightComponent::getColor()
{
	return mColor;
}

bool SpotLightComponent::getActive()
{
	return isActive;
}

float SpotLightComponent::getIntensity()
{
	return mIntensity;
}

float SpotLightComponent::getRange()
{
	return mRange;
}

float SpotLightComponent::getUAngle()
{
	return mUAngle;
}

float SpotLightComponent::getPAngle()
{
	return mPAngle;
}

void SpotLightComponent::setColor(const XMFLOAT4 color)
{
	mColor = color;
}

void SpotLightComponent::setActive(const bool state)
{
	isActive = state;
}

void SpotLightComponent::setIntensity(const float intensity)
{
	mIntensity = intensity;
}

void SpotLightComponent::setRange(const float range)
{
	mRange = range;
}

void SpotLightComponent::setUAngle(const float uAngle)
{
	mUAngle = uAngle;
}

void SpotLightComponent::setPAngle(const float pAngle)
{
	mPAngle = pAngle;
}
