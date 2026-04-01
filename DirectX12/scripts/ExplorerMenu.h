#pragma once
#include "Menu.h"

class PlayerManager;
class ItemManager;

class ExplorerMenu : public Menu {
public:
	ExplorerMenu(TownScene& scene, float zDepth);
	void selectedAct() override;

private:
	void prepareCraftExplorer();
	void craftExplorer(int index);

	std::vector<std::string> mExplorer;

	PlayerManager* mPlayerManager;
	ItemManager* mItemManager;


};


