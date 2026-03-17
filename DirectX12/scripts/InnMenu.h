#pragma once
#include "TownManager.h"

class InnMenu : public Menu {
public:
	InnMenu(Game* game, float zDepth);
	void selectedAct() override;

private:
	void stay();
	void save();
};

