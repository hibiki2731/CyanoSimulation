#include "ForgeMenu.h"
#include "TownScene.h"
#include "AudioManager.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include <fstream>
#include "json.hpp"

ForgeMenu::ForgeMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ForgeMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
}


void ForgeMenu::selectedAct()
{
	if (mSelectedIndex < mWeapons.size()) craftWeapons(mSelectedIndex);
	else craftArmer(mSelectedIndex - mWeapons.size());
}

void ForgeMenu::prepareCraftItems()
{
	mWeapons.emplace_back("WOODEN_SORD");
	mArmers.emplace_back("CLOTHES");
	mMaxIndex = mWeapons.size() + mArmers.size();
}

void ForgeMenu::craftWeapons(int index)
{
	//リソースを消費
	const auto& weaponData = mItemManager.getWeaponData(mWeapons[index]);
	for (int i = 0; i < weaponData.costResourceID.size(); i++) {
		int possessedResource = mItemManager.getResourceNum(weaponData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (weaponData.price[i] > possessedResource) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}

		//所持リソースを消費リソース分減らす
		mItemManager.subResource(weaponData.costResourceID[i], weaponData.price[i]);
	}

	mScene.getGame().getAudioManager().playSE("UI_ENTER");

	//インベントリにアイテムを追加
	mPlayerManager.addWeapon(weaponData.id);
}

void ForgeMenu::craftArmer(int index)
{
	//リソースを消費
	const auto& armerData = mItemManager.getArmerData(mArmers[index]);
	for (int i = 0; i < armerData.costResourceID.size(); i++) {
		int possessedResource = mItemManager.getResourceNum(armerData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (armerData.price[i] > possessedResource) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}

		//所持リソースを消費リソース分減らす
		mItemManager.subResource(armerData.costResourceID[i], armerData.price[i]);
	}

	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	//インベントリにアイテムを追加
	mPlayerManager.addArmer(armerData.id);
}

