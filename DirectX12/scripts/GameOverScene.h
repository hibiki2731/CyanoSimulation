#pragma once
#include "Scene.h"

class Game;

class GameOverScene : public Scene
{
public:
	GameOverScene(Game& game);

	void onEnter() override;
	void onExit() override;

	const std::string getName() const override {
		return "GAMEOVER";
	};

};

class GameOverUI : public Actor
{
public:
	GameOverUI(GameOverScene& scene);
	DECLARE_CLASS_NAME(GameOverUI)

	void inputActor() override;
	void updateActor() override;

private:
	void startTransit();
	bool movingToTitle;
};

