#pragma once
#include "Menu.h"

class MainMenu : public Menu {
public:
	MainMenu(Game& game,TownManager& townManager, float zDepth);
	void selectedAct() override;
};

