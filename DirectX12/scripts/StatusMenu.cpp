#include "input.h"
#include "StatusMenu.h"
#include "Game.h"
#include "TownScene.h"
#include "MessageWindow.h"
#include <memory>
#include "PlayerManager.h"
#include "TextComponent.h"
#include "MyUtility.h"
#include "SpriteComponent.h"
#include "AudioManager.h"
#include <fstream>
#include "json.hpp"

EquipWeaponMenu::EquipWeaponMenu(TownScene& scene, float zDepth)
	: Menu(scene, "EquipWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	mMaxIndex = mPlayerManager.getPlayerData().weaponInventory.size();
	mScrollOffset = 0;

	//ファイルの読み込み
	std::ifstream spriteFile("assets\\data\\spriteData.json");
	nlohmann::json spriteJson;
	spriteFile >> spriteJson;
	std::ifstream textFile("assets\\data\\textData.json");
	nlohmann::json textJson;
	textFile >> textJson;

	//所持武器のテキストを作成
	std::string structName = "EquipWeaponMenuScrollText";
	auto weaponText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = 0; i < showWeaponNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedWeaponIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	weaponText->setText(message);
	weaponText->setBaseLine(textJson[structName]["x"].get<float>(), textJson[structName]["y"].get<float>());
	weaponText->setFontSize(textJson[structName]["fontSize"].get<float>());
	weaponText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	weaponText->setLineSpace(textJson[structName]["lineSpace"].get<float>());
	weaponText->showText();
#ifdef _DEBUG
	weaponText->activateControll(structName);
#endif

	mTextComponent = weaponText.get();
	addComponent(std::move(weaponText));

	mArrowMoveLength = textJson[structName]["lineSpace"].get<float>();

	//スクロールインジケーター
	//下矢印
	structName = "EquipWeaponMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	mDownArrow = downArrow.get();
	addComponent(std::move(downArrow));

	//上矢印
	structName = "EquipWeaponMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	mUpArrow = upArrow.get();
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "EquipWeaponMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().x;
	float height = maxHeight * MaxShowWeaponNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowWeaponNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(height, scrollBar->getSpriteSize().y));
#ifdef _DEBUG
	scrollBar->activateControll(structName);
#endif
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));

}

void EquipWeaponMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	mPlayerManager.equipWeapon(mSelectedIndex);
	refreshText();
}

void EquipWeaponMenu::updateMenu()
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

void EquipWeaponMenu::inputMenu()
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

int EquipWeaponMenu::getScrollOffset()
{
	return mScrollOffset;
}

void EquipWeaponMenu::refreshText()
{
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = mScrollOffset; i < mScrollOffset + showWeaponNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedWeaponIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	mTextComponent->setText(message);
	mTextComponent->showText();

}

EquipArmerMenu::EquipArmerMenu(TownScene& scene, float zDepth)
	: Menu(scene, "EquipArmerMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	mMaxIndex = mPlayerManager.getPlayerData().armerInventory.size();
	mScrollOffset = 0;

	std::string structName = "EquipArmerMenuScrollText";
	auto armerText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	armerText->loadFileAndCreate(structName);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = 0; i < showArmerNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedArmerIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	armerText->setText(message);
	armerText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	armerText->showText();
#ifdef _DEBUG
	armerText->activateControll(structName);
#endif
	mArrowMoveLength = armerText->getLineSpace();
	mTextComponent = armerText.get();
	addComponent(std::move(armerText));


	//スクロールインジケーター
	//下矢印
	structName = "EquipArmerMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->loadFileAndCreate(structName);
	mDownArrow = downArrow.get();
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	addComponent(std::move(downArrow));

	//上矢印
	structName = "EquipArmerMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	mUpArrow = upArrow.get();
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "EquipArmerMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().x;
	float height = maxHeight * MaxShowArmerNum / mPlayerManager.getPlayerData().armerInventory.size();
	if (mPlayerManager.getPlayerData().armerInventory.size() < MaxShowArmerNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().armerInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(height, scrollBar->getSpriteSize().y));
	mScrollBar = scrollBar.get();
#ifdef _DEBUG
	scrollBar->activateControll(structName);
#endif
	addComponent(std::move(scrollBar));
}

void EquipArmerMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	mPlayerManager.equipArmer(mSelectedIndex);
	refreshText();
}

void EquipArmerMenu::updateMenu()
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

void EquipArmerMenu::inputMenu()
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
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		mSelectedIndex++;
		if (mSelectedIndex > mScrollOffset + MaxShowArmerNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void EquipArmerMenu::refreshText()
{
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = mScrollOffset; i < mScrollOffset + showArmerNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedArmerIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	mTextComponent->setText(message);
	mTextComponent->showText();
}

StatusMenu::StatusMenu(TownScene& scene, float zDepth) : Menu(scene, "StatusMenu", zDepth)
{
	mMaxIndex = 2;
}

StatusMenu::~StatusMenu()
{
	mScene.exitStatusMenu();
}

void StatusMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("UI_WINDOW_OPEN");
	switch(mSelectedIndex) {
	case 0: {
		auto weaponMenu = std::make_unique<EquipWeaponMenu>(mScene, 48.0f);
		mScene.addActor(std::move(weaponMenu));
		break;
	}
	case 1: {
		auto armerMenu = std::make_unique<EquipArmerMenu>(mScene, 48.0f);
		mScene.addActor(std::move(armerMenu));
		break;
	}
	}
}

