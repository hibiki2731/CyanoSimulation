#pragma once
#include "Menu.h"

class PlayerManager;
class TextComponent;

class EquipWeaponMenu : public Menu {
public:
	EquipWeaponMenu(TownScene& scene, class StatusMenu& menu, float zDepth);
	DECLARE_CLASS_NAME(EquipWeaponMenu)

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

	int getScrollOffset();
private:
	void refreshText();
	void updateDescriptor();
	PlayerManager& mPlayerManager;
	TextComponent* mTextComponent;
	SpriteComponent* mScrollBar;
	SpriteComponent* mEquipIcon;
	TextComponent* mDescriptor;
	float mScrollBarMoveLength;

	int mScrollOffset;
	const int MaxShowWeaponNum = 6;

	class ItemManager& mItemManager;
	class StatusMenu& mStatusMenu;
};

class EquipArmerMenu : public Menu {
public:
	EquipArmerMenu(TownScene& scene, class StatusMenu& menu, float zDepth);
	DECLARE_CLASS_NAME(EquipArmerMenu)

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void refreshText();
	void updateDescriptor();
	PlayerManager& mPlayerManager;
	TextComponent* mTextComponent;
	SpriteComponent* mScrollBar;
	SpriteComponent* mEquipIcon;
	TextComponent* mDescriptor;
	float mScrollBarMoveLength;

	int mScrollOffset;
	const int MaxShowArmerNum = 6;
	
	class ItemManager& mItemManager;
	class StatusMenu& mStatusMenu;
};
class StatusMenu : public Menu
{
public:
	StatusMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(StatusMenu)

	~StatusMenu();
	void selectedAct() override;
	void applyStatus();

private:
	PlayerManager& mPlayerManager;
	TextComponent* mStatusText;
};

