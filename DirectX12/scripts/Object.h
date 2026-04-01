#pragma once
#include "Actor.h"
#include "AssetManager.h"

class Object : public Actor
{
public:
	Object(Scene& scene, const std::string& meshID, float x, float y);
};

