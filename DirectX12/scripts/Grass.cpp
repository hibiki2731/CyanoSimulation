#include "Grass.h"
#include "MeshComponent.h"
#include "Game.h"
#include "json.hpp"
#include "ItemManager.h"
#include <fstream>

Resource::Resource(Game& game, const std::string& meshID, const std::string& resourceID, float x, float y) : 
	Actor(game, x, y), mResourceID(resourceID)
{
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	addComponent(std::move(mesh));
}

void Resource::updateActor() {
}

void Resource::inputActor(){}
