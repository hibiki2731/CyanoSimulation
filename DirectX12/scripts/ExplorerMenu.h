#pragma once
#include "Menu.h"

class PlayerManager;
class ItemManager;
class TextComponent;
class SpriteComponent;

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
	void showCraftCost();
	void showResource();
	void showToolEffect();

	std::vector<std::string> mTools;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mToolText;
	SpriteComponent* mScrollBar;
	TextComponent* mCostText;
	TextComponent* mResourceText;
	TextComponent* mToolEffectText;

	int mScrollOffset;
	const int MaxShowToolNum = 5;
	float mScrollBarMoveLength;



};


