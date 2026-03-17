#pragma once
#include "TownManager.h"

class PlayerManager;
class ItemManager;

class ExplorerMenu : public Menu {
public:
	ExplorerMenu(Game* game, float zDepth);
	void selectedAct() override;

private:
	void prepareCraftExplorer();
	void craftExplorer(int index);

	std::vector<std::string> mExplorer;

	PlayerManager* mPlayerManager;
	ItemManager* mItemManager;


};


