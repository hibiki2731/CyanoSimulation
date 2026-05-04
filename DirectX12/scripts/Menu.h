#pragma once
#include "Object.h"
#include <string>

class TownScene;
class SpriteComponent;


class Menu : public Object
{
public:
	Menu(TownScene& scene, const std::string& windowName, float zDepth);
	//input
	virtual	void inputMenu();
	virtual void updateActor() override;
	virtual void selectedAct() {};
	virtual void updateMenu() {};

	void applyComponentLabel() override;

protected:
	int mMaxIndex;	//メニューの最大インデックス
	int mSelectedIndex;	//選択されているメニューのインデックス
	float mArrowMoveLength;	//矢印の移動距離
	SpriteComponent* mArrow;
	float mArrowDefaultPosition;
	TownScene& mScene;
};
