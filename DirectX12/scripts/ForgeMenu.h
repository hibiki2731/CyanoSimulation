#pragma once
#include "Menu.h"
#include <vector>
#include <string>

class PlayerManager;
class ItemManager;

class ForgeMenu : public Menu {
public:

	ForgeMenu(TownScene& scene, float zDepth);
	void selectedAct() override;

private:
	void prepareCraftItems();
	void craftWeapons(int index);
	void craftArmer(int index);

	std::vector<std::string> mWeapons;
	std::vector<std::string> mArmers;

	PlayerManager* mPlayerManager;
	ItemManager* mItemManager;

};

