#pragma once
#include "Actor.h"
class PointLight :
    public Actor
{
public:
	PointLight(Game* game);

    void updateActor() override;
    void inputActor() override;
};

