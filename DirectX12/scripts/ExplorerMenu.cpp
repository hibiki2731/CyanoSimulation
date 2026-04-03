#include "TownScene.h"
#include "ExplorerMenu.h"
#include "AudioManager.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"

ExplorerMenu::ExplorerMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ExplorerShopMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
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
	const auto& explorerData = mItemManager.getExplorerData(mExplorer[index]);
	for (int i = 0; i < explorerData.costResourceID.size(); i++) {
		int possessedResource = mItemManager.getResourceNum(explorerData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (explorerData.price[i] > possessedResource) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}

		//所持リソースを消費リソース分減らす
		mItemManager.subResource(explorerData.costResourceID[i], explorerData.price[i]);
	}

	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	//インベントリにアイテムを追加
	mPlayerManager.addExplorer(explorerData.id);

}


