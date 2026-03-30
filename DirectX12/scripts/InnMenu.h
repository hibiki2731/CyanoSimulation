#pragma once
#include "Menu.h"

class InnMenu : public Menu {
public:
	InnMenu(Game& game, TownManager& townManager, float zDepth);
	void selectedAct() override;

private:
	void stay();
	void save();
};

