#pragma once
#include "Actor.h"

class Anime2DComponent;

class UI : public Actor
{
public:
	UI(Scene& scene);
	~UI() {};

	void updateActor() override;

private:
	Anime2DComponent* mAnime;
	int mTextureIndex;
	int counter;
};

