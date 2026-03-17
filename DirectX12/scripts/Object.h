#pragma once
#include "Actor.h"
#include "AssetManager.h"

class Object : public Actor
{
public:
	Object(Game* game, MeshName meshName, float x, float y);
};

