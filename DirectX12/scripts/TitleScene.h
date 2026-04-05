#pragma once
#include "Scene.h"
#include "Actor.h"


class TitleScene : public Scene
{
public:
	TitleScene(class Game& game);
	void onEnter() override;
	void onExit() override;
	void inputScene() override;

	const std::string& getName() const override {
		return "TITLE";
	};

private:
	class ItemManager& mItemManager;
	class PlayerManager& mPlayerManager;
};


class TitleUI : public Actor
{
public:
	TitleUI(TitleScene& scene);
};
