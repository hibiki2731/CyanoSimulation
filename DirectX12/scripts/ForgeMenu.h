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
	DECLARE_CLASS_NAME(ArmerForgeMenu)
	void applyComponentLabel() override;

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void prepareCraftItems();
	void craftArmer(int index);
	void refreshText();
	void showCraftCost();
	void showResource();
	void showArmerEffect();

	std::vector<std::string> mArmers;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mArmerText;
	SpriteComponent* mScrollBar;
	TextComponent* mCostText;
	TextComponent* mResourceText;
	TextComponent* mArmerEffectText;

	const int MaxShowArmerNum = 7;
	float mScrollBarMoveLength;
	int mScrollOffset;
};

class WeaponMenu : public Menu {
public:
	WeaponMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(WeaponForgeMenu)
	void applyComponentLabel() override;

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void prepareCraftItems();
	void craftWeapon(int index);
	void refreshText();
	void showCraftCost();
	void showResource();
	void showWeaponEffect();

	std::vector<std::string> mWeapons;

	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mWeaponText;
	SpriteComponent* mScrollBar;
	TextComponent* mCostText;
	TextComponent* mResourceText;
	TextComponent* mWeaponEffectText;

	const int MaxShowWeaponNum = 7;
	float mScrollBarMoveLength;
	int mScrollOffset;
};

class ForgeMenu : public Menu {
public:
	ForgeMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(ForgeMenu)

	void selectedAct() override;
	void inputMenu() override;
	void updateActor() override;
	void applyComponentLabel();

private:
	TextComponent* mDescriptor;
	float mDescriptorDefaultZ;

	void updateDescriptor();
};

