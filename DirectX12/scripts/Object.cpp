#include "Object.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Game.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"

Object::Object(Scene& scene, const std::string& meshID, float x, float y) : Actor(scene, x, y)
{
	mPosition = XMFLOAT3(x, 0, y);

	//メッシュコンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	//所有権をActorに渡す
	addComponent(std::move(mesh));

}

void Object::setPointLight(const PointLightDescription& lightData)
{
	auto light = std::make_unique<PointLightComponent>(*this);
	light->setColor(lightData.color);
	light->setIntensity(lightData.intensity);
	light->setRange(lightData.range);
	light->setActive(true);
	addComponent(std::move(light));
}

void Object::setSpotLight(const SpotLightDescription& lightData)
{
	auto light = std::make_unique<SpotLightComponent>(*this);
	light->setColor(lightData.color);
	light->setIntensity(lightData.intensity);
	light->setRange(lightData.range);
	light->setUAngle(lightData.attAngleRange);
	light->setPAngle(lightData.angleRange);
	light->setActive(true);
	addComponent(std::move(light));
}
