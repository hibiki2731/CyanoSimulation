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

	void updateComponent() override;

	//ゲッター
	XMFLOAT4 getPosition() const;
	XMFLOAT4 getColor() const;
	bool getActive() const;
	float getRange() const;
	float getIntensity() const;
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

	class Graphic& mGraphic;

#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};

