#include "ForgeMenu.h"
#include "TownScene.h"
#include "AudioManager.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include <fstream>
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "json.hpp"
#include "MyUtility.h"
#include "input.h"

ForgeMenu::ForgeMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ForgeMenu", zDepth)
{
	mMaxIndex = 2;
	addComponentLabel("descriptor", "TextComponent");
	applyComponentLabel();
}

void ForgeMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("UI_WINDOW_OPEN");
	switch (mSelectedIndex) {
	case 0: {
		auto weaponMenu = std::make_unique<WeaponMenu>(mScene, 80.0f);
		mScene.addActor(std::move(weaponMenu));
		break;
	}
	case 1: {
		auto armerMenu = std::make_unique<ArmerMenu>(mScene, 80.0f);
		mScene.addActor(std::move(armerMenu));
		break;
	}
	}

}

void ForgeMenu::inputMenu()
{
	if (mArrow) {
		if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
			if (mSelectedIndex <= 0) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex--;
			mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}

		if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
			if (mSelectedIndex >= mMaxIndex - 1) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex++;
			mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}
	}
}

void ForgeMenu::updateActor()
{
	if (mDescriptor) {
		//メニューがアクティブでないとき
		if (mScene.getCurrentMenu() != this) {
			mDescriptor->setPosZ(200.0f);
		}
		else
			mDescriptor->setPosZ(mDescriptorDefaultZ);
	}
}

void ForgeMenu::applyComponentLabel()
{
	mDescriptor = static_cast<TextComponent*>(mComponentLabels["descriptor"].pComponent);
	if (mDescriptor) {
		mDescriptorDefaultZ = mDescriptor->getPosZ();
		updateDescriptor();
	}
}

void ForgeMenu::updateDescriptor()
{
	if (!mDescriptor) return;

	std::wstring text;
	switch (mSelectedIndex) {
	case 0:
		text = L"武器を作製します。\n";
		break;
	case 1:
		text = L"防具を作製します。\n";
		break;
	}

	mDescriptor->setText(text);
}

ArmerMenu::ArmerMenu(TownScene& scene, float zDepth)
	:Menu(scene, "ForgeArmerMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
	mScrollOffset = 0;	

	addComponentLabel("armerText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("resourceText", "TextComponent");
	addComponentLabel("armerEffectText", "TextComponent");
	addComponentLabel("costText", "TextComponent");

	applyComponentLabel();
}

void ArmerMenu::applyComponentLabel()
{
	//購入可能な武器と防具のテキストを作成
	mArmerText = static_cast<TextComponent*>(mComponentLabels["armerText"].pComponent);
	if (mArmerText) {
		std::wstring armerText;
		for (const auto& armerID : mArmers) {
			const auto& armerData = mItemManager.getArmerData(armerID);
			armerText += Utility::stringToWString(armerData.name) + L"\n";
		}
		if (armerText.size() == 0) armerText = L"なし\n";
		mArmerText->setText(armerText);

		//矢印の移動距離を設定
		mArrowMoveLength = mArmerText->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowArmerNum / mArmers.size();
		if (mArmers.size() < MaxShowArmerNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mArmers.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//所持リソースのテキストを作成
	mResourceText = static_cast<TextComponent*>(mComponentLabels["resourceText"].pComponent);
	if(mResourceText) showResource();

	//装備の効果
	mArmerEffectText = static_cast<TextComponent*>(mComponentLabels["armerEffectText"].pComponent);
	if(mArmerEffectText) showArmerEffect();

	//装備作製にかかるコスト
	mCostText = static_cast<TextComponent*>(mComponentLabels["costText"].pComponent);
	if(mCostText) showCraftCost();

}

void ArmerMenu::selectedAct()
{
	craftArmer(mSelectedIndex);
}

void ArmerMenu::updateMenu()
{
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowArmerNum - 1) {		
		mScrollOffset++;
		mScrollBar->movePosition(XMFLOAT2(0.0f, mScrollBarMoveLength));
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		mScrollBar->movePosition(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		refreshText();
	}
}

void ArmerMenu::inputMenu()
{
	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex--;
		showCraftCost();
		showArmerEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex++;
		showCraftCost();
		showArmerEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex > mScrollOffset + MaxShowArmerNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void ArmerMenu::prepareCraftItems()
{
	for (auto& data : mItemManager.getArmerData()) {
		if (data.second.inPossession) continue;
		mArmers.emplace_back(data.second.id);
	}

	mMaxIndex = mArmers.size();
}

void ArmerMenu::craftArmer(int index)
{
	if (mArmers.size() == 0) {
		mScene.getGame().getAudioManager().playSE("UI_CANCEL");
		return;
	}

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

	mScene.getGame().getAudioManager().playSE("FORGE");

	//陳列からアイテムを削除
	mArmers.erase(mArmers.begin() + index);
	mItemManager.setArmerPossession(armerData.id, true);
	//インベントリにアイテムを追加
	mPlayerManager.addArmer(armerData.id);
	//防具を装備
	mPlayerManager.equipArmer(mPlayerManager.getPlayerData().armerInventory.size() - 1);

	//各テキストの更新
	refreshText();
	showArmerEffect();
	showCraftCost();
	showResource();
	mScene.updateStatusWindow();
}

void ArmerMenu::refreshText()
{
	//テキストの更新
	std::wstring armerText = L"";
	int showArmerNum = min(mArmers.size(), MaxShowArmerNum);
	for (int i = mScrollOffset; i < mScrollOffset + showArmerNum; i++) {
		const auto& armerData = mItemManager.getWeaponData(mArmers[i]);
		armerText += Utility::stringToWString(armerData.name) + L"\n";
	}
	if (armerText.size() == 0) armerText = L"なし\n";
	mArmerText->setText(armerText);

	//インデックスの更新
	mMaxIndex--;
	if (mSelectedIndex >= mMaxIndex && mSelectedIndex > 0) {
		mSelectedIndex--;
		if (mSelectedIndex < mScrollOffset) {
			mScrollOffset--;
			mScrollBar->movePosition(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		}
		else mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}
}

void ArmerMenu::showCraftCost()
{
	//防具のデータを取得
	if (mArmers.size() == 0) {
		mCostText->setText(L" ");
		return;
	}
	const auto& armerData = mItemManager.getArmerData(mArmers[mSelectedIndex]);
	//防具の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < armerData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(armerData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" " + std::to_wstring(armerData.price[i]) + L"  ";
	}
	costText += L"\n";
	mCostText->setText(costText);
}

void ArmerMenu::showResource()
{
	//リソースデータの取得
	auto resourceData = mItemManager.getResourceData();
	//所持リソースの表示
	std::wstring resourceText = L"所持リソース\n";
	for (auto& data : resourceData) {
		if (data.second.num <= 0) continue;
		resourceText += Utility::stringToWString(data.second.name) + L" : " + std::to_wstring(mItemManager.getResourceNum(data.second.id)) + L"  ";
	}
	resourceText += L"\n";
	mResourceText->setText(resourceText);
}

void ArmerMenu::showArmerEffect()
{
	//防具データを取得
	if (mArmers.size() == 0) {
		mArmerEffectText->setText(L" ");
		return;
	}
	const auto& armerData = mItemManager.getArmerData(mArmers[mSelectedIndex]);
	//防具の効果を表示
	std::wstring description = Utility::stringToWString(armerData.description);
	mArmerEffectText->setText(description);
}

WeaponMenu::WeaponMenu(TownScene& scene, float zDepth)
	:Menu(scene, "ForgeWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
	mScrollOffset = 0;

	addComponentLabel("weaponText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("resourceText", "TextComponent");
	addComponentLabel("weaponEffectText", "TextComponent");
	addComponentLabel("costText", "TextComponent");

	applyComponentLabel();
}

void WeaponMenu::applyComponentLabel()
{
	//購入可能な武器と防具のテキストを作成
	mWeaponText = static_cast<TextComponent*>(mComponentLabels["weaponText"].pComponent);
	if (mWeaponText) {
		std::wstring weaponText;
		for (const auto& weaponID : mWeapons) {
			const auto& weaponData = mItemManager.getWeaponData(weaponID);
			weaponText += Utility::stringToWString(weaponData.name) + L"\n";
		}
		if (weaponText.size() == 0) weaponText = L"なし\n";
		mWeaponText->setText(weaponText);

		//矢印の移動距離を設定
		mArrowMoveLength = mWeaponText->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowWeaponNum / mWeapons.size();
		if (mWeapons.size() < MaxShowWeaponNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mWeapons.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//所持リソースのテキストを作成
	mResourceText = static_cast<TextComponent*>(mComponentLabels["resourceText"].pComponent);
	if(mResourceText) showResource();

	//装備の効果
	mWeaponEffectText = static_cast<TextComponent*>(mComponentLabels["weaponEffectText"].pComponent);
	if(mWeaponEffectText) showWeaponEffect();

	//装備作製にかかるコスト
	mCostText = static_cast<TextComponent*>(mComponentLabels["costText"].pComponent);
	if(mCostText) showCraftCost();
}

void WeaponMenu::selectedAct()
{
	craftWeapon(mSelectedIndex);
}

void WeaponMenu::updateMenu()
{
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowWeaponNum - 1) {		
		mScrollOffset++;
		mScrollBar->movePosition(XMFLOAT2(0.0f, mScrollBarMoveLength));
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		mScrollBar->movePosition(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		refreshText();
	}
}

void WeaponMenu::inputMenu()
{
	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex--;
		showWeaponEffect();
		showCraftCost();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex++;
		showWeaponEffect();
		showCraftCost();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex > mScrollOffset + MaxShowWeaponNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void WeaponMenu::prepareCraftItems()
{
	for (auto& data : mItemManager.getWeaponData()) {
		if (data.second.inPossession) continue;
		mWeapons.emplace_back(data.second.id);
	}

	mMaxIndex = mWeapons.size();
}

void WeaponMenu::craftWeapon(int index)
{
	if (mWeapons.size() == 0) {
		mScene.getGame().getAudioManager().playSE("UI_CANCEL");
		return;
	}

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

	mScene.getGame().getAudioManager().playSE("FORGE");

	//陳列からアイテムを削除
	mWeapons.erase(mWeapons.begin() + index);
	mItemManager.setWeaopnPossession(weaponData.id, true);
	//インベントリにアイテムを追加
	mPlayerManager.addWeapon(weaponData.id);
	refreshText();

	//武器の装備
	mPlayerManager.equipWeapon(mPlayerManager.getPlayerData().weaponInventory.size() - 1);
	//各テキストを更新
	showWeaponEffect();
	showCraftCost();
	showResource();
	mScene.updateStatusWindow();
}

void WeaponMenu::refreshText()
{
	//テキストの更新
	std::wstring weaponText = L"";
	int showItemNum = min(mWeapons.size(), MaxShowWeaponNum);
	for (int i = mScrollOffset; i < mScrollOffset + showItemNum; i++) {
		const auto& weaponData = mItemManager.getWeaponData(mWeapons[i]);
		weaponText += Utility::stringToWString(weaponData.name) + L"\n";
	}
	if (weaponText.size() == 0) weaponText = L"なし\n";
	mWeaponText->setText(weaponText);

	//インデックスの更新
	mMaxIndex--;
	if (mSelectedIndex >= mMaxIndex && mSelectedIndex > 0) {
		mSelectedIndex--;
		if (mSelectedIndex < mScrollOffset) {
			mScrollOffset--;
			mScrollBar->movePosition(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		}
		else mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}
}

void WeaponMenu::showCraftCost()
{
	//武器のデータを取得
	if (mWeapons.size() == 0) {
		std::wstring costText = L" ";
		mCostText->setText(costText);
		return;
	}
	const auto& weaponData = mItemManager.getWeaponData(mWeapons[mSelectedIndex]);
	//武器の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < weaponData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(weaponData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" " + std::to_wstring(weaponData.price[i]) + L"  ";
	}
	costText += L"\n";
	mCostText->setText(costText);
}

void WeaponMenu::showResource()
{
	//リソースデータの取得
	auto resourceData = mItemManager.getResourceData();
	//所持リソースの表示
	std::wstring resourceText = L"所持リソース\n";
	for (auto& data : resourceData) {
		if (data.second.num <= 0) continue;
		resourceText += Utility::stringToWString(data.second.name) + L" : " + std::to_wstring(mItemManager.getResourceNum(data.second.id)) + L"  ";
	}
	resourceText += L"\n";
	mResourceText->setText(resourceText);
}

void WeaponMenu::showWeaponEffect()
{
	//武器データを取得
	if (mWeapons.size() == 0) {
		std::wstring description = L" ";
		mWeaponEffectText->setText(description);
		return;
	}

	const auto& weaponData = mItemManager.getWeaponData(mWeapons[mSelectedIndex]);
	//武器の効果を表示
	std::wstring description = Utility::stringToWString(weaponData.description);
	mWeaponEffectText->setText(description);
}
