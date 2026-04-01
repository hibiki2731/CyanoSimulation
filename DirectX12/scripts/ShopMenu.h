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

private:
	void buyItem(int indes);
	void prepareSaleItem();

	std::vector<std::string> mSaleItem;
	ItemManager* mItemManager;
	PlayerManager* mPlayerManager;
};

