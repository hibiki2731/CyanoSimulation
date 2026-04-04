#pragma once
#include <vector>
#include <string>
#include "Menu.h"

class ItemManager;
class PlayerManager;
struct ItemData;

class ShopMenu : public Menu {
public:
	ShopMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void buyItem(int indes);
	void prepareSaleItem();
	void refreshText();

	std::vector<std::string> mSaleItem;
	ItemManager& mItemManager;
	PlayerManager& mPlayerManager;
	class TextComponent* mShopText;
	class SpriteComponent* mScrollBar;

	int mScrollOffset;
	const int MaxShowItemNum = 7;
	float mScrollBarMoveLength;

};

