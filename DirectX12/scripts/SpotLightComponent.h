#pragma once
#include "Component.h"
#include <DirectXMath.h>
using namespace DirectX;

class SpotLightComponent : public Component
{
public:
	SpotLightComponent(Actor* owner, int updateOrder = 100);

	void inputComponent() override;
	void updateComponent() override;
	void endProccess() override;

	//ゲッター
	XMFLOAT4 getPosition();
	XMFLOAT4 getDirection();
	XMFLOAT4 getColor();
	bool getActive();
	float getIntensity();
	float getRange();
	float getUAngle();
	float getPAngle();

	//セッター
	void setColor(const XMFLOAT4 color);
	void setActive(const bool state);
	void setIntensity(const float intensity);
	void setRange(const float range);
	void setUAngle(const float uAngle);
	void setPAngle(const float pAngle);

private:

	XMFLOAT4 mPosition; //xyz:座標
	XMFLOAT4 mDirection; //xyz:向き
	XMFLOAT4 mColor; //xyz:rgb w:α値
	float isActive;
	float mIntensity;
	float mRange;
	float mUAngle; //角度減衰が起こらない範囲 radianにする
	float mPAngle; //ライトがあたる範囲 radianにする
};

