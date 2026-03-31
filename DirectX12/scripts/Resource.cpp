#include "Resource.h"
#include "MeshComponent.h"
#include "Game.h"
#include "json.hpp"
#include "ItemManager.h"
#include "DungeonScene.h"
#include <fstream>

Resource::Resource(DungeonScene& scene, const std::string& meshID, const std::string& resourceID, float x, float y) : 
	Actor(scene, x, y), mResourceID(resourceID)
{
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	addComponent(std::move(mesh));
}

void Resource::updateActor() {
}

void Resource::inputActor(){}
