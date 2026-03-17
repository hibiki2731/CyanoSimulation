#pragma once
#include "TownManager.h"

class PlayerManager;
class TextComponent;

class EquipWeaponMenu : public Menu {
public:
	EquipWeaponMenu(Game* game, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

	int getScrollOffset();
private:
	void refreshText();
	PlayerManager* mPlayerManager;
	TextComponent* mTextComponent;

	int mScrollOffset;
	const int MaxShowWeaponNum = 5;
};

class EquipArmerMenu : public Menu {
public:
	EquipArmerMenu(Game* game, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void refreshText();
	PlayerManager* mPlayerManager;
	TextComponent* mTextComponent;

	int mScrollOffset;
	const int MaxShowArmerNum = 3;
};
class StatusMenu : public Menu
{
public:
	StatusMenu(Game* game, float zDepth);
	~StatusMenu();
	void selectedAct() override;

};

