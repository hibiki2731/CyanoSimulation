#pragma once
#include <string>
#include <stack>
#include "Scene.h"
#include "Actor.h"

class Game;
class Menu;

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

	//ステータスウィンドウの更新
	void updateStatusWindow();

	//getter
	const std::string getName() const override {
		return "TOWN";
	}
	Menu* getCurrentMenu();


private:
	bool isSelected;
	std::stack<Menu*> mMenuStack; //アクティブなメニューを管理
	class TownUI* mUI;
};

