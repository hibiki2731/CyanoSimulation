#include "PointLight.h"
#include "PointLightComponent.h"
#include "timer.h"
#include "Game.h"
#include <windows.h>

PointLight::PointLight(Game* game) : Actor(game)
{
	auto light = std::make_unique<PointLightComponent>(this);
	light->setActive(true);
	light->setColor(XMFLOAT4(0.1f, 0.3f, 1.0f, 1.0f));
	light->setIntensity(3.0f);
	light->setRange(2.0f);
	addComponent(std::move(light));
}

void PointLight::updateActor()
{
}

void PointLight::inputActor()
{
}
