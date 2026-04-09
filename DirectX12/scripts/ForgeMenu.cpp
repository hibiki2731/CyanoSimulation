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

	//ファイル読み込み
	std::ifstream spriteFile("assets\\data\\spriteData.json");
	nlohmann::json spriteJson;
	spriteFile >> spriteJson;
	std::ifstream textFile("assets\\data\\textData.json");
	nlohmann::json textJson;
	textFile >> textJson;

	//購入可能な武器と防具のテキストを作成
	std::string structName = "ForgeArmerMenuScrollText";
	std::wstring armerText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(textJson[structName]["fontSize"].get<float>());
	textComponent->setLineSpace(textJson[structName]["lineSpace"].get<float>());
	textComponent->setPosition(textJson[structName]["x"].get<float>(), textJson[structName]["y"].get<float>());
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& armerID : mArmers) {
		const auto& armerData = mItemManager.getArmerData(armerID);
		armerText += Utility::stringToWString(armerData.name) + L"\n";
	}
	if (armerText.size() == 0) armerText = L"なし\n";
	textComponent->setText(armerText);
	textComponent->showText();
#ifdef _DEBUG
	textComponent->activateControll(structName);
#endif
	mArmerText = textComponent.get();
	addComponent(std::move(textComponent));

	//矢印の移動距離を設定
	mArrowMoveLength = textJson[structName]["lineSpace"].get<float>();

	//スクロールバー
	//下矢印
	structName = "ForgeArmerMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	downArrow->create(spriteJson[structName]["filePath"].get<std::string>());
	downArrow->setPosition(XMFLOAT3(spriteJson[structName]["x"].get<float>(), spriteJson[structName]["y"].get<float>(), zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(spriteJson[structName]["width"].get<float>(), spriteJson[structName]["height"].get<float>()));
	downArrow->setRotation(spriteJson[structName]["rotation"].get<float>());
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	addComponent(std::move(downArrow));

	//上矢印
	structName = "ForgeArmerMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	upArrow->create(spriteJson[structName]["filePath"].get<std::string>());
	upArrow->setPosition(XMFLOAT3(spriteJson[structName]["x"].get<float>(), spriteJson[structName]["y"].get<float>(), zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(spriteJson[structName]["width"].get<float>(), spriteJson[structName]["height"].get<float>()));
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "ForgeArmerMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = spriteJson[structName]["height"].get<float>();
	float height = maxHeight * MaxShowArmerNum / mArmers.size();
	if (mArmers.size() < MaxShowArmerNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mArmers.size();
	scrollBar->setSpriteSize(XMFLOAT2(scrollBar->getSpriteSize().x, height));
#ifdef _DEBUG
	scrollBar->activateControll(structName);
#endif
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));

	//所持リソース表示用のキャンバス
	structName = "ResourceCanvas";
	auto resourceCanvas = std::make_unique<SpriteComponent>(*this, zDepth);
	resourceCanvas->loadFileAndCreate(structName);
#ifdef _DEBUG
	resourceCanvas->activateControll(structName);
#endif
	addComponent(std::move(resourceCanvas));

	//所持リソースのテキストを作成
	structName = "ResourceText";
	auto resourceText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	resourceText->loadFileAndCreate(structName);
	resourceText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	resourceText->activateControll(structName);
#endif
	mResourceText = resourceText.get();
	showResource();
	addComponent(std::move(resourceText));

	//装備の効果
	structName = "EffectCanvas";
	auto armerEffectCanvas = std::make_unique<SpriteComponent>(*this, zDepth );
	armerEffectCanvas->loadFileAndCreate(structName);

#ifdef _DEBUG
	armerEffectCanvas->activateControll(structName);
#endif
	addComponent(std::move(armerEffectCanvas));

	//装備の効果　テキスト	
	structName = "EffectText";
	auto armerEffectText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	armerEffectText->loadFileAndCreate(structName);
	armerEffectText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	armerEffectText->activateControll(structName);
#endif
	mArmerEffectText = armerEffectText.get();
	showArmerEffect();
	addComponent(std::move(armerEffectText));

	//装備作製にかかるコスト
	structName = "CostText";
	auto costText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	costText->loadFileAndCreate(structName);
	costText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	costText->activateControll(structName);
#endif
	mCostText = costText.get();
	showCraftCost();
	addComponent(std::move(costText));
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
		showCraftCost();
		showArmerEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
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
	if (mArmers.size() == 0)return;

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
	showArmerEffect();
	showCraftCost();
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
	mArmerText->showText();

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
		mCostText->showText();
		return;
	}
	const auto& armerData = mItemManager.getArmerData(mArmers[mSelectedIndex]);
	//防具の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < armerData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(armerData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" : " + std::to_wstring(armerData.price[i]);
	}
	costText += L"\n";
	mCostText->setText(costText);
	mCostText->showText();
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
	mResourceText->showText();
}

void ArmerMenu::showArmerEffect()
{
	//防具データを取得
	if (mArmers.size() == 0) {
		mArmerEffectText->setText(L" ");
		mArmerEffectText->showText();
		return;
	}
	const auto& armerData = mItemManager.getArmerData(mArmers[mSelectedIndex]);
	//防具の効果を表示
	std::wstring description = Utility::stringToWString(armerData.description);
	mArmerEffectText->setText(description);
	mArmerEffectText->showText();
}

WeaponMenu::WeaponMenu(TownScene& scene, float zDepth)
	:Menu(scene, "ForgeWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftItems();
	mScrollOffset = 0;

	//ファイル読み込み
	std::ifstream spriteFile("assets\\data\\spriteData.json");
	nlohmann::json spriteJson;
	spriteFile >> spriteJson;
	std::ifstream textFile("assets\\data\\textData.json");
	nlohmann::json textJson;
	textFile >> textJson;

	std::string structName;
	//購入可能な武器と防具のテキストを作成
	structName = "ForgeWeaponMenuScrollText";
	std::wstring weaponText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(textJson[structName]["fontSize"].get<float>());
	textComponent->setLineSpace(textJson[structName]["lineSpace"].get<float>());
	textComponent->setPosition(textJson[structName]["x"].get<float>(), textJson[structName]["y"].get<float>());
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& weaponID : mWeapons) {
		const auto& weaponData = mItemManager.getWeaponData(weaponID);
		weaponText += L"・" + Utility::stringToWString(weaponData.name) + L"\n";
	}
	if (weaponText.size() == 0) weaponText = L"なし\n";
	textComponent->setText(weaponText);
	textComponent->showText();
#ifdef _DEBUG
	textComponent->activateControll(structName);
#endif
	mWeaponText = textComponent.get();
	addComponent(std::move(textComponent));

	//インジケーターの移動距離を設定
	mArrowMoveLength = textJson[structName]["lineSpace"].get<float>();

	//スクロールバー
	//下矢印
	structName = "ForgeWeaponMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	downArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	addComponent(std::move(downArrow));

	//上矢印
	structName = "ForgeWeaponMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	upArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "ForgeWeaponMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().y;
	float height = maxHeight * MaxShowWeaponNum / mWeapons.size();
	if (mWeapons.size() < MaxShowWeaponNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mWeapons.size();
	scrollBar->setSpriteSize(XMFLOAT2(scrollBar->getSpriteSize().x, height));
#ifdef _DEBUG
	scrollBar->activateControll(structName);
#endif
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));

	//所持リソース表示用のキャンバス
	structName = "ResourceCanvas";
	auto resourceCanvas = std::make_unique<SpriteComponent>(*this, zDepth);
	resourceCanvas->loadFileAndCreate(structName);
#ifdef _DEBUG
	resourceCanvas->activateControll(structName);
#endif
	addComponent(std::move(resourceCanvas));

	//所持リソースのテキストを作成
	structName = "ResourceText";
	auto resourceText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	resourceText->loadFileAndCreate(structName);
	resourceText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	resourceText->activateControll(structName);
#endif
	mResourceText = resourceText.get();
	showResource();
	addComponent(std::move(resourceText));

	//装備の効果
	structName = "EffectCanvas";
	auto weaponEffectCanvas = std::make_unique<SpriteComponent>(*this, zDepth );
	weaponEffectCanvas->loadFileAndCreate(structName);

#ifdef _DEBUG
	weaponEffectCanvas->activateControll(structName);
#endif
	addComponent(std::move(weaponEffectCanvas));

	//装備の効果　テキスト	
	structName = "EffectText";
	auto weaponEffectText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	weaponEffectText->loadFileAndCreate(structName);
	weaponEffectText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	weaponEffectText->activateControll(structName);
#endif
	mWeaponEffectText = weaponEffectText.get();
	showWeaponEffect();
	addComponent(std::move(weaponEffectText));

	//装備作製にかかるコスト
	structName = "CostText";
	auto costText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	costText->loadFileAndCreate(structName);
	costText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	costText->activateControll(structName);
#endif
	mCostText = costText.get();
	showCraftCost();
	addComponent(std::move(costText));
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
		showWeaponEffect();
		showCraftCost();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
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
	if (mWeapons.size() == 0) return;

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
	showWeaponEffect();
	showCraftCost();
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
	mWeaponText->showText();

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
		mCostText->showText();
		return;
	}
	const auto& weaponData = mItemManager.getWeaponData(mWeapons[mSelectedIndex]);
	//武器の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < weaponData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(weaponData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" : " + std::to_wstring(weaponData.price[i]);
	}
	costText += L"\n";
	mCostText->setText(costText);
	mCostText->showText();
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
	mResourceText->showText();
}

void WeaponMenu::showWeaponEffect()
{
	//武器データを取得
	if (mWeapons.size() == 0) {
		std::wstring description = L" ";
		mWeaponEffectText->setText(description);
		mWeaponEffectText->showText();
		return;
	}

	const auto& weaponData = mItemManager.getWeaponData(mWeapons[mSelectedIndex]);
	//武器の効果を表示
	std::wstring description = Utility::stringToWString(weaponData.description);
	mWeaponEffectText->setText(description);
	mWeaponEffectText->showText();
}
