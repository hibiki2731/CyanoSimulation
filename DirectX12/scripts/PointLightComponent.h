#pragma once
#include <DirectXMath.h>
#include "Component.h"
#include "Definition.h"
using namespace DirectX;

class PointLightComponent : public Component
{
public:
	PointLightComponent(Actor& owner, int updateOrder = 100);
	DECLARE_COMPONENT_NAME(PointLightComponent)
	void loadFromJson(const nlohmann::json& json) override;

	void endProcess() override;
	void updateComponent() override;

	//ゲッター
	XMFLOAT4 getPosition();
	XMFLOAT4 getColor();
	bool getActive();
	float getRange();
	float getIntensity();
	//セッター
	void setColor(const XMFLOAT4& color);
	void setActive(const bool state);
	void setIntensity(const float intensity);
	void setRange(const float range);
	void setOffsetPos(const XMFLOAT4& offset);

private:
	bool isActive;
	float mRange;
	float mIntensity;
	XMFLOAT4 mColor;
	XMFLOAT4 mPosition;
	XMFLOAT4 mOffsetPos;

#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};

