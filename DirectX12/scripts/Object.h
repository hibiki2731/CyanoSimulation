#pragma once
#include "Actor.h"
#include "AssetManager.h"

class Object : public Actor
{
public:
	Object(Scene& scene, const std::string& meshID, float x, float y);

	struct PointLightDescription {
		XMFLOAT4 offsetPos = { 0.0f, 0.0f, 0.0f, 0.0f };
		XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
		float range = 1.0f;
	};

	struct SpotLightDescription {
		XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
		float range = 1.0f;
		float attAngleRange = 0.0f;
		float angleRange = 0.0f;
	};
	
	void setPointLight(const PointLightDescription& lightData);
	void setSpotLight(const SpotLightDescription& lightData);
};

