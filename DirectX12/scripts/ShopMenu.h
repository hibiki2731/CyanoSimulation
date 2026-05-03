#pragma once
#include <vector>
#include <string>
#include "Menu.h"

class ItemManager;
class PlayerManager;
struct ItemData;
class TextComponent;
class SpriteComponent;

class ShopMenu : public Menu {
public:
	ShopMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(ShopMenu)

	void applyComponentLabel() override;

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;

private:
	void buyItem(int indes);
	void prepareSaleItem();
	void refreshText();
	void showItemEffect();
	void showItemCost();
	void showResource();

	std::vector<std::string> mSaleItem;
	ItemManager& mItemManager;
	PlayerManager& mPlayerManager;
	TextComponent* mShopText;
	SpriteComponent* mScrollBar;
	TextComponent* mItemEffectText;
	TextComponent* mCostText;
	TextComponent* mResourceText;

	int mScrollOffset;
	const int MaxShowItemNum = 7;
	float mScrollBarMoveLength;

};

