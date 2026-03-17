#include "Grass.h"
#include "MeshComponent.h"

Grass::Grass(Game* game, float x, float y) : Actor(game, x, y)
{
	auto mesh = std::make_unique<MeshComponent>(this);
	mesh->create(MeshName::GRASS);
	addComponent(std::move(mesh));
}

void Grass::updateActor() {
}

void Grass::inputActor(){}
