#pragma once
#include "Menu.h"

class PlayerManager;
class ItemManager;

class ExplorerMenu : public Menu {
public:
	ExplorerMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void prepareCraftExplorer();
	void craftExplorer(int index);
	void refreshText();

	std::vector<std::string> mTools;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	class TextComponent* mToolText;
	class SpriteComponent* mScrollBar;

	int mScrollOffset;
	const int MaxShowToolNum = 5;
	float mScrollBarMoveLength;



};


