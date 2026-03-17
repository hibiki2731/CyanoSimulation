#pragma once
#include "Actor.h"

class Anime2DComponent;

class UI : public Actor
{
public:
	UI(Game* game);
	~UI() {};

	void updateActor() override;

private:
	Anime2DComponent* mAnime;
	int mTextureIndex;
	int counter;
};

