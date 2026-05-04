#pragma once
#include "Menu.h"

class SpriteComponent;
class TextComponent;

class MainMenu : public Menu {
public:
	MainMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(MainMenu)

	void applyComponentLabel() override;
	void updateMenu() override;
	void updateActor() override;
	void inputMenu() override;

	void selectedAct() override;
private:
	void startTransit();
	void updateDescriptor();

	TextComponent* mDescriptor;
	float mDescriptorDefaultZ;
	bool movingToDungeon;
};

