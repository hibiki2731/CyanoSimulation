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
	BackGround(Scene& scene);
};

class TownScene :public Scene
{
public:
	TownScene(Game& game);

	void onEnter() override;
	void onExit() override;
	void updateScene() override;
	void inputScene() override;
	
	void pushMenu(Menu* menu);
	void popMenu();

	void exitStatusMenu();

	const std::string& getName() const override {
		return "TOWN";
	}

private:
	BackGround* mBg;
	bool isSelected;
	bool isSelecetdStatus;
	bool isStatusMenu;
	std::stack<Menu*> mMenuStack; //アクティブなメニューを管理
};

