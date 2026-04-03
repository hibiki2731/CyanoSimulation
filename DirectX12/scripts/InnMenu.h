#pragma once
#include "Menu.h"

class InnMenu : public Menu {
public:
	InnMenu(TownScene& scene, float zDepth);
	void selectedAct() override;

private:
	void stay();
	void save();
};

