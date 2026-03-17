#include "ExplorerMenu.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"

ExplorerMenu::ExplorerMenu(Game* game, float zDepth) : Menu(game, "ExplorerShopMenu", zDepth)
{
	mPlayerManager = game->getPlayerManager();
	mItemManager = game->getItemManager();

	prepareCraftExplorer();
}

void ExplorerMenu::selectedAct()
{
	craftExplorer(mSelectedIndex);
}

void ExplorerMenu::prepareCraftExplorer()
{
	mExplorer.emplace_back("BOOTS");
	mMaxIndex = mExplorer.size();
}

void ExplorerMenu::craftExplorer(int index)
{
	//リソースを消費
	const auto& explorerData = mItemManager->getExplorerData(mExplorer[index]);
	for (int i = 0; i < explorerData.costResourceID.size(); i++) {
		int possessedResource = mItemManager->getResourceNum(explorerData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (explorerData.price[i] > possessedResource) return;

		//所持リソースを消費リソース分減らす
		mItemManager->subResource(explorerData.costResourceID[i], explorerData.price[i]);
	}

	//インベントリにアイテムを追加
	mPlayerManager->addExplorer(explorerData.id);

}


