#include "UI.h"
#include "Game.h"
#include "Component.h"
#include "Anime2DComponent.h"
#include "SpriteComponent.h"

UI::UI(Game* game) : Actor(game)
{
	mTextureIndex = 0;
	counter = 0;
	auto anime = std::make_unique<Anime2DComponent>(this);
	anime->create("assets\\josei_04_akamafu\\josei_04_akamafu\\PNG\\josei_04_.png", 4);
	setScale({ 1.0f / getGame()->getGraphic()->getAspect() * 0.5f, 1.0f * 1.1f, 1.0f });
	mAnime = anime.get();
	addComponent(std::move(anime));

	auto sprite = std::make_unique<SpriteComponent>(this);
	sprite->create("assets\\white.png");
	addComponent(std::move(sprite));
}

void UI::updateActor()
{
	counter++;
	if (counter % 30 == 0){
		mTextureIndex++;
		if (mTextureIndex >= mAnime->getTextureNum()) {
			mTextureIndex = 0;
		}
		mAnime->setTextureIndex(mTextureIndex);
		
	}
}
