#include <windows.h>
#include "Math.h"
#include "timer.h"
#include "PointLightComponent.h"
#include "Actor.h"
#include "Game.h"
#include "json.hpp"
#include "myJson.h"
#include "Scene.h"

PointLightComponent::PointLightComponent(Actor& owner, int updateOrder) : Component(owner, updateOrder)
{
	isActive = false;
	mOffsetPos = { 0.0f, 0.0f, 0.0f, 0.0f };
	mPosition.x = mOwner.getPosition().x;
	mPosition.y = mOwner.getPosition().y;
	mPosition.z = mOwner.getPosition().z;
	mPosition.w = 1.0f;
	mIntensity = 1.0f;
	mRange = 1.0f;
	mColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mOwner.getScene().addPointLight(this);
}

void PointLightComponent::loadFromJson(const nlohmann::json& json)
{			
	setOffsetPos(json.value("lightOffsetPos", XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	setColor(json.value("lightColor", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	setIntensity(json.value("intensity", 1.0f));
	setRange(json.value("range", 1.0f));
	setActive(true);
}

void PointLightComponent::endProcess()
{
	//Gameからライトを削除
	mOwner.getScene().removePointLight(this);
}

void PointLightComponent::updateComponent()
{
	if (isActive) {
		//位置の更新
		mPosition.x = mOwner.getPosition().x;
		mPosition.y = mOwner.getPosition().y;
		mPosition.z = mOwner.getPosition().z;
		mPosition = mPosition + mOffsetPos;
	}
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

void PointLightComponent::setColor(const XMFLOAT4& color)
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

void PointLightComponent::setOffsetPos(const XMFLOAT4& offset)
{
	mOffsetPos = offset;
}
