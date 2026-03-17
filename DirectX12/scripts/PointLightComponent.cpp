#include <windows.h>
#include "Math.h"
#include "timer.h"
#include "PointLightComponent.h"
#include "Actor.h"
#include "Game.h"

PointLightComponent::PointLightComponent(Actor* owner, int updateOrder) : Component(owner, updateOrder)
{
	isActive = false;
	mPosition.w = 1.0f;
	mIntensity = 1.0f;
	mRange = 1.0f;
	mColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mOwner->getGame()->addPointLight(this);
}

void PointLightComponent::inputComponent()
{
}

void PointLightComponent::updateComponent()
{
	if (isActive) {
		mPosition.x = mOwner->getPosition().x;
		mPosition.y = mOwner->getPosition().y;
		mPosition.z = mOwner->getPosition().z;
	}
}

void PointLightComponent::endProccess()
{
	//Gameからライトを削除
	mOwner->getGame()->removePointLight(this);
}

XMFLOAT4 PointLightComponent::getPosition()
{
	return mPosition;
}

XMFLOAT4 PointLightComponent::getColor()
{
	return mColor;
}

bool PointLightComponent::getActive()
{
	return isActive;
}

float PointLightComponent::getRange()
{
	return mRange;
}

float PointLightComponent::getIntensity()
{
	return mIntensity;
}

void PointLightComponent::setColor(const XMFLOAT4 color)
{
	mColor = color;
}

void PointLightComponent::setActive(const bool state)
{
	isActive = state;
}

void PointLightComponent::setIntensity(const float intensity)
{
	mIntensity = intensity;
}

void PointLightComponent::setRange(const float range)
{
	mRange = range;
}
