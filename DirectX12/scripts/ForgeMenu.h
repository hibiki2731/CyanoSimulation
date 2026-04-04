#pragma once
#include "Menu.h"
#include <vector>
#include <string>

class json;
class PlayerManager;
class ItemManager;
class TextComponent;

class ArmerMenu : public Menu {
public:
	ArmerMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void prepareCraftItems();
	void craftArmer(int index);
	void refreshText();

	std::vector<std::string> mArmers;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mArmerText;
	SpriteComponent* mScrollBar;

	const int MaxShowArmerNum = 7;
	float mScrollBarMoveLength;
	int mScrollOffset;
};

class WeaponMenu : public Menu {
public:
	WeaponMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void prepareCraftItems();
	void craftWeapon(int index);
	void refreshText();

	std::vector<std::string> mWeapons;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mWeaponText;
	SpriteComponent* mScrollBar;

	const int MaxShowWeaponNum = 7;
	float mScrollBarMoveLength;
	int mScrollOffset;
};

class ForgeMenu : public Menu {
public:
	ForgeMenu(TownScene& scene, float zDepth);
	void selectedAct() override;
};

