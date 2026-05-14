#pragma once
#include "Component.h"
#include <DirectXMath.h>
using namespace DirectX;

class SpotLightComponent : public Component
{
public:
	SpotLightComponent(Actor& owner, int updateOrder = 100);
	DECLARE_COMPONENT_NAME(SpotLightComponent)

	void updateComponent() override;

	//ゲッター
	XMFLOAT4 getPosition() const;
	XMFLOAT4 getDirection() const;
	XMFLOAT4 getColor() const;
	bool getActive() const;
	float getIntensity() const;
	float getRange() const;
	float getUAngle() const;
	float getPAngle() const;

	//セッター
	void setColor(const XMFLOAT4& color);
	void setActive(const bool state);
	void setIntensity(const float intensity);
	void setRange(const float range);
	void setUAngle(const float uAngle);
	void setPAngle(const float pAngle);
	void setOffsetPos(const XMFLOAT4& offsetPos);

private:

	XMFLOAT4 mPosition; //xyz:座標
	XMFLOAT4 mOffsetPos;
	XMFLOAT4 mDirection; //xyz:向き
	XMFLOAT4 mColor; //xyz:rgb w:α値
	float isActive;
	float mIntensity;
	float mRange;
	float mUAngle; //角度減衰が起こらない範囲 radianにする
	float mPAngle; //ライトがあたる範囲 radianにする

	class Graphic& mGraphic;
};

