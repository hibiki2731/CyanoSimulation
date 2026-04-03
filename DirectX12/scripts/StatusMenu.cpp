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

EquipWeaponMenu::EquipWeaponMenu(TownScene& scene, float zDepth)
	: Menu(scene, "EquipWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	mMaxIndex = mPlayerManager.getPlayerData().weaponInventory.size();
	mScrollOffset = 0;

	auto weaponText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = 0; i < showWeaponNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedWeaponIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	float fontSize =40.0f;
	float lineSpace = 8.0f;
	weaponText->setText(message);
	weaponText->setBaseLine(110.0f, 175.0f);
	weaponText->setFontSize(fontSize);
	weaponText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	weaponText->setLineSpace(lineSpace);
	weaponText->showText();
	mTextComponent = weaponText.get();
	addComponent(std::move(weaponText));

	mArrowMoveLength = fontSize + lineSpace;

	//スクロールインジケーター
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + showWeaponNum * 48.0f - 8.0f, zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	downArrow->setRotation(XM_PI);
	mDownArrow = downArrow.get();
	addComponent(std::move(downArrow));

	//上矢印
	auto upArrow = std::make_unique<SpriteComponent>(*this);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->setPosition(XMFLOAT3(80.0f, 175.0f, zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	mUpArrow = upArrow.get();
	addComponent(std::move(upArrow));

	//スクロールバー
	auto scrollBar = std::make_unique<SpriteComponent>(*this);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->setPosition(XMFLOAT3(80.0f, 175.0f + 30.0f, zDepth - 0.5f));
	scrollBar->setBordarSize(0.0f);
	float arrowDistance = 48.0f * showWeaponNum - 38.0f;
	float height = arrowDistance * MaxShowWeaponNum / mPlayerManager.getPlayerData().weaponInventory.size();
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
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
		mScrollBar->movePositon(XMFLOAT2(0.0f, mScrollBarMoveLength));
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		mScrollBar->movePositon(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		refreshText();
	}
}

void EquipWeaponMenu::inputMenu()
{


	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex == 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePositon(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex == mMaxIndex - 1) return;
		mSelectedIndex++;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex > mScrollOffset + MaxShowWeaponNum  - 1) return;
		mArrow->movePositon(XMFLOAT2(0.0f, mArrowMoveLength));
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
	mPlayerManager = scene.getGame().getPlayerManager();
	mMaxIndex = mPlayerManager.getPlayerData().armerInventory.size();
	mScrollOffset = 0;

	auto armerText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = 0; i < showArmerNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedArmerIndex) message += L"<E> ";
		else message += L"        ";
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	float fontSize =40.0f;
	float lineSpace = 8.0f;
	armerText->setText(message);
	armerText->setBaseLine(110.0f, 175.0f);
	armerText->setFontSize(fontSize);
	armerText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	armerText->setLineSpace(lineSpace);
	armerText->showText();
	mTextComponent = armerText.get();
	addComponent(std::move(armerText));

	mArrowMoveLength = fontSize + lineSpace;

	//スクロールインジケーター
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + showArmerNum * 48.0f - 8.0f, zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	downArrow->setRotation(XM_PI);
	mDownArrow = downArrow.get();
	addComponent(std::move(downArrow));

	//上矢印
	auto upArrow = std::make_unique<SpriteComponent>(*this);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->setPosition(XMFLOAT3(80.0f, 175.0f, zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	mUpArrow = upArrow.get();
	addComponent(std::move(upArrow));

	//スクロールバー
	auto scrollBar = std::make_unique<SpriteComponent>(*this);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->setPosition(XMFLOAT3(80.0f, 175.0f + 30.0f, zDepth - 0.5f));
	scrollBar->setBordarSize(0.0f);
	float arrowDistance = 48.0f * showArmerNum - 38.0f;
	float height = arrowDistance * MaxShowArmerNum / mPlayerManager.getPlayerData().armerInventory.size();
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().armerInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
	mScrollBar = scrollBar.get();
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
		mScrollBar->movePositon(XMFLOAT2(0.0f, mScrollBarMoveLength));
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		mScrollBar->movePositon(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		refreshText();
	}
}

void EquipArmerMenu::inputMenu()
{

	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex == 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePositon(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex == mMaxIndex - 1) return;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		mSelectedIndex++;
		if (mSelectedIndex > mScrollOffset + MaxShowArmerNum  - 1) return;
		mArrow->movePositon(XMFLOAT2(0.0f, mArrowMoveLength));
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

