#pragma once
#include "Actor.h"

class Grass : public Actor
{
public:
	Grass(Game* game, float x, float y);

	void updateActor() override;
	void inputActor() override;

private:
};

