#pragma once
#include "Menu.h"

class SpriteComponent;
class TextComponent;

class MainMenu : public Menu {
public:
	MainMenu(TownScene& scene, float zDepth);
	void selectedAct() override;

};

