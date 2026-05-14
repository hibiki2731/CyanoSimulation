#include <windows.h>
#include "Math.h"
#include "timer.h"
#include "PointLightComponent.h"
#include "Actor.h"
#include "Game.h"
#include "json.hpp"
#include "myJson.h"
#include "Scene.h"
#include "Graphic.h"

PointLightComponent::PointLightComponent(Actor& owner, int updateOrder)
	: Component(owner, updateOrder),
	mGraphic(owner.getScene().getGame().getGraphic())
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
}

void PointLightComponent::loadFromJson(const nlohmann::json& json)
{			
	setOffsetPos(json.value("lightOffsetPos", XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)));
	setColor(json.value("lightColor", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	setIntensity(json.value("intensity", 1.0f));
	setRange(json.value("range", 1.0f));
	setActive(true);
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

	mGraphic.updatePointLight(*this);
}

XMFLOAT4 PointLightComponent::getPosition() const
{
	return mPosition;
}

XMFLOAT4 PointLightComponent::getColor() const
{
	return mColor;
}

bool PointLightComponent::getActive() const
{
	return isActive;
}

float PointLightComponent::getRange() const
{
	return mRange;
}

float PointLightComponent::getIntensity() const
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
