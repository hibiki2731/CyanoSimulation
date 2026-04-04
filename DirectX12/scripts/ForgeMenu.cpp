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
}


void ForgeMenu::selectedAct()
{
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

ArmerMenu::ArmerMenu(TownScene& scene, float zDepth)
	:Menu(scene, "ForgeArmerMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
	mScrollOffset = 0;	

	static float fontSize =40.0f;
	static float lineSpace = 8.0f;

	//購入可能な武器と防具のテキストを作成
	std::wstring armerText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(fontSize);
	textComponent->setLineSpace(lineSpace);
	textComponent->setBaseLine(mPosition.x + 60.0f, mPosition.y + 75.0f);
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& armerID : mArmers) {
		const auto& armerData = mItemManager.getArmerData(armerID);
		armerText += Utility::stringToWString(armerData.name) + L"\n";
	}
	if (armerText.size() == 0) armerText = L"なし\n";
	textComponent->setText(armerText);
	textComponent->showText();
	mArmerText = textComponent.get();
	addComponent(std::move(textComponent));

	//矢印の移動距離を設定
	mArrowMoveLength = fontSize + lineSpace;

	//スクロールバー
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + MaxShowArmerNum * 48.0f - 8.0f, zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	downArrow->setRotation(XM_PI);
	addComponent(std::move(downArrow));

	//上矢印
	auto upArrow = std::make_unique<SpriteComponent>(*this);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->setPosition(XMFLOAT3(80.0f, 175.0f, zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	addComponent(std::move(upArrow));

	//スクロールバー
	auto scrollBar = std::make_unique<SpriteComponent>(*this);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->setPosition(XMFLOAT3(80.0f, 175.0f + 30.0f, zDepth - 0.5f));
	scrollBar->setBordarSize(10.0f);
	float arrowDistance = 48.0f * MaxShowArmerNum - 38.0f;
	float height = arrowDistance * MaxShowArmerNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowArmerNum) height = arrowDistance;
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));
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
	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
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

	//陳列からアイテムを削除
	mArmers.erase(mArmers.begin() + index);
	mItemManager.setArmerPossession(armerData.id, true);
	//インベントリにアイテムを追加
	mPlayerManager.addArmer(armerData.id);
	refreshText();
}

void ArmerMenu::refreshText()
{
	//テキストの更新
	std::wstring armerText = L"";
	for (const auto& armerID : mArmers) {
		const auto& armerData = mItemManager.getArmerData(armerID);
		armerText += L"・" + Utility::stringToWString(armerData.name) + L"\n";
	}
	if (armerText.size() == 0) armerText = L"なし\n";
	mArmerText->setText(armerText);
	mArmerText->showText();

	//インデックスの更新
	mMaxIndex--;
	if (mSelectedIndex >= mMaxIndex && mSelectedIndex > 0) {
		mSelectedIndex--;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}
}

WeaponMenu::WeaponMenu(TownScene& scene, float zDepth)
	:Menu(scene, "ForgeWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
	mScrollOffset = 0;

	static float fontSize =40.0f;
	static float lineSpace = 8.0f;

	//購入可能な武器と防具のテキストを作成
	std::wstring weaponText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(fontSize);
	textComponent->setLineSpace(lineSpace);
	textComponent->setBaseLine(mPosition.x + 60.0f, mPosition.y + 75.0f);
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& weaponID : mWeapons) {
		const auto& weaponData = mItemManager.getWeaponData(weaponID);
		weaponText += L"・" + Utility::stringToWString(weaponData.name) + L"\n";
	}
	if (weaponText.size() == 0) weaponText = L"なし\n";
	textComponent->setText(weaponText);
	textComponent->showText();
	mWeaponText = textComponent.get();
	addComponent(std::move(textComponent));

	//インジケーターの移動距離を設定
	mArrowMoveLength = fontSize + lineSpace;

	//スクロールバー
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + MaxShowWeaponNum * 48.0f - 8.0f, zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	downArrow->setRotation(XM_PI);
	addComponent(std::move(downArrow));

	//上矢印
	auto upArrow = std::make_unique<SpriteComponent>(*this);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->setPosition(XMFLOAT3(80.0f, 175.0f, zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	addComponent(std::move(upArrow));

	//スクロールバー
	auto scrollBar = std::make_unique<SpriteComponent>(*this);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->setPosition(XMFLOAT3(80.0f, 175.0f + 30.0f, zDepth - 0.5f));
	scrollBar->setBordarSize(10.0f);
	float arrowDistance = 48.0f * MaxShowWeaponNum - 38.0f;
	float height = arrowDistance * MaxShowWeaponNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowWeaponNum) height = arrowDistance;
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));
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
	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
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

	//陳列からアイテムを削除
	mWeapons.erase(mWeapons.begin() + index);
	mItemManager.setWeaopnPossession(weaponData.id, true);
	//インベントリにアイテムを追加
	mPlayerManager.addWeapon(weaponData.id);
	refreshText();
}

void WeaponMenu::refreshText()
{
	//テキストの更新
	std::wstring weaponText = L"";
	for (const auto& weaponID : mWeapons) {
		const auto& weaponData = mItemManager.getWeaponData(weaponID);
		weaponText += Utility::stringToWString(weaponData.name) + L"\n";
	}
	if (weaponText.size() == 0) weaponText = L"なし\n";
	mWeaponText->setText(weaponText);
	mWeaponText->showText();

	//インデックスの更新
	mMaxIndex--;
	if (mSelectedIndex >= mMaxIndex && mSelectedIndex > 0) {
		mSelectedIndex--;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}
}
