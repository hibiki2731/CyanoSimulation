#pragma once
#include <vector>
#include <string>
#include "TownManager.h"

class ItemManager;
class PlayerManager;
struct ItemData;

class ShopMenu : public Menu {
public:
	ShopMenu(Game* game, float zDepth);
	void selectedAct() override;

private:
	void buyItem(int indes);
	void prepareSaleItem();

	std::vector<std::string> mSaleItem;
	ItemManager* mItemManager;
	PlayerManager* mPlayerManager;
};

