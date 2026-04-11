#pragma once
#include "Menu.h"

class SpriteComponent;
class TextComponent;

class MainMenu : public Menu {
public:
	MainMenu(TownScene& scene, float zDepth);

	void updateMenu() override;

	void selectedAct() override;
private:
	void startTransit();

	bool movingToDungeon;
};

