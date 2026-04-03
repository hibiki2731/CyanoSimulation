#pragma once
#include "Menu.h"

class MainMenu : public Menu {
public:
	MainMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
};

