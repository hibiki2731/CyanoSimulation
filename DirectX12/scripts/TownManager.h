#pragma once
#include <string>
#include <stack>
#include "Actor.h"

class Game;
class SpriteComponent;

class Menu : public Actor
{
public:
	Menu(Game* game, std::string windowName, float zDepth);
	//input
	virtual	void inputMenu();
	virtual void selectedAct() {};
	virtual void updateMenu() {};

protected:
	int mMaxIndex;	//メニューの最大インデックス
	int mSelectedIndex;	//選択されているメニューのインデックス
	float mArrowMoveLength;	//矢印の移動距離
	SpriteComponent* mArrow;

private:
	void initComponent(std::string windowName, float zDepth);

};

class MainMenu : public Menu {
public:
	MainMenu(Game* game, float zDepth);
	void selectedAct() override;
};

class BackGround : public Actor
{
public:
	BackGround(Game* game);
};

class TownManager
{
public:
	TownManager(Game* game);

	void update();
	void input();
	
	void pushMenu(Menu* menu);
	void popMenu();

	void exitStatusMenu();

private:
	Game* mGame;
	BackGround* mBg;
	bool isTown;
	bool isSelected;
	bool isSelecetdStatus;
	bool isStatusMenu;
	std::stack<Menu*> mMenuStack; //アクティブなメニューを管理
};

