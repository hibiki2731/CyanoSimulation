#include "ShopMenu.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"

ShopMenu::ShopMenu(Game* game, float zDepth) : Menu(game, "ShopMenu", zDepth)
{
	mItemManager = game->getItemManager();
	mPlayerManager = game->getPlayerManager();
	prepareSaleItem();
}

void ShopMenu::selectedAct()
{
	buyItem(mSelectedIndex);
}

void ShopMenu::prepareSaleItem()
{
	mSaleItem.emplace_back("POSION");
	mSaleItem.emplace_back("BOMB");
	mMaxIndex = mSaleItem.size();
}

void ShopMenu::buyItem(int index) {
	//リソースを消費
	const auto& itemData = mItemManager->getItemData(mSaleItem[index]);
	for (int i = 0; i < itemData.costResourceID.size(); i++) {
		int possessedResource = mItemManager->getResourceNum(itemData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (itemData.price[i] > possessedResource) return;

		//所持リソースを消費リソース分減らす
		mItemManager->subResource(itemData.costResourceID[i], itemData.price[i]);
	}

	//インベントリにアイテムを追加
	mPlayerManager->addInventory(itemData.id);
}


