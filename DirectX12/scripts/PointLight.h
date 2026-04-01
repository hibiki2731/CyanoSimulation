#pragma once
#include "Actor.h"
class PointLight :
    public Actor
{
public:
	PointLight(Scene& scene);

    void updateActor() override;
    void inputActor() override;
};

