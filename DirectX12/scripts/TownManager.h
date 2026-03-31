#pragma once
#include <string>
#include <stack>
#include "Scene.h"
#include "Actor.h"

class Game;
class Menu;

class BackGround : public Actor
{
public:
	BackGround(Game& game);
};

class TownManager :public Scene
{
public:
	TownManager(Game& game);

	void onEnter() override;
	void onExit() override;
	void update() override;
	void input() override;
	
	void pushMenu(Menu* menu);
	void popMenu();

	void exitStatusMenu();

	const std::string& getName() const override {
		return "TOWN";
	}

private:
	Game& mGame;
	BackGround* mBg;
	bool isTown;
	bool isSelected;
	bool isSelecetdStatus;
	bool isStatusMenu;
	std::stack<Menu*> mMenuStack; //アクティブなメニューを管理

	std::vector<Actor* > mActors;
};

