#pragma once
#include "Actor.h"
#include <string>

class TownManager;
class SpriteComponent;


class Menu : public Actor
{
public:
	Menu(Game& game, TownManager& townManager, std::string windowName, float zDepth);
	//input
	virtual	void inputMenu();
	virtual void selectedAct() {};
	virtual void updateMenu() {};

protected:
	int mMaxIndex;	//メニューの最大インデックス
	int mSelectedIndex;	//選択されているメニューのインデックス
	float mArrowMoveLength;	//矢印の移動距離
	SpriteComponent* mArrow;

	TownManager& mTownManager;

private:
	void initComponent(std::string windowName, float zDepth);

};
