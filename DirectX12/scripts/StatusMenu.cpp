#include "input.h"
#include "StatusMenu.h"
#include "Game.h"
#include "TownManager.h"
#include "MessageWindow.h"
#include <memory>
#include "PlayerManager.h"
#include "TextComponent.h"
#include "MyUtility.h"
#include "SpriteComponent.h"
#include "AudioManager.h"

EquipWeaponMenu::EquipWeaponMenu(Game& game, TownManager& townManager, float zDepth) : Menu(game, townManager, "EquipWeaponMenu", zDepth)
{
	mPlayerManager = game.getPlayerManager();
	mMaxIndex = mPlayerManager->getPlayerData().weaponInventory.size();
	mScrollOffset = 0;

	auto weaponText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	std::wstring message;
	const auto& inventory = mPlayerManager->getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = 0; i < showWeaponNum; i++) {
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	float fontSize =40.0f;
	float lineSpace = 8.0f;
	weaponText->setText(message);
	weaponText->setBaseLine(80.0f, 175.0f);
	weaponText->setFontSize(fontSize);
	weaponText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	weaponText->setLineSpace(lineSpace);
	weaponText->showText();
	mTextComponent = weaponText.get();
	addComponent(std::move(weaponText));

	mArrowMoveLength = fontSize + lineSpace;

}

void EquipWeaponMenu::selectedAct()
{
	mScene.getAudioManager()->playSE("UI_ENTER");
	mPlayerManager->equipWeapon(mSelectedIndex);
}

void EquipWeaponMenu::updateMenu()
{
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowWeaponNum - 1) {		
		mScrollOffset++;
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		refreshText();
	}
}

void EquipWeaponMenu::inputMenu()
{


	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex == 0) return;
		mSelectedIndex--;
		mScene.getAudioManager()->playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePositon(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex == mMaxIndex - 1) return;
		mSelectedIndex++;
		mScene.getAudioManager()->playSE("UI_MOVE1");
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
	const auto& inventory = mPlayerManager->getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = mScrollOffset; i < mScrollOffset + showWeaponNum; i++) {
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	mTextComponent->setText(message);
	mTextComponent->showText();

}

EquipArmerMenu::EquipArmerMenu(Game& game, TownManager& townManager, float zDepth) : Menu(game, townManager, "EquipArmerMenu", zDepth)
{
	mPlayerManager = game.getPlayerManager();
	mMaxIndex = mPlayerManager->getPlayerData().armerInventory.size();
	mScrollOffset = 0;

	auto armerText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	std::wstring message;
	const auto& inventory = mPlayerManager->getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = 0; i < showArmerNum; i++) {
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	float fontSize =40.0f;
	float lineSpace = 8.0f;
	armerText->setText(message);
	armerText->setBaseLine(80.0f, 175.0f);
	armerText->setFontSize(fontSize);
	armerText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	armerText->setLineSpace(lineSpace);
	armerText->showText();
	mTextComponent = armerText.get();
	addComponent(std::move(armerText));

	mArrowMoveLength = fontSize + lineSpace;
}

void EquipArmerMenu::selectedAct()
{
	mScene.getAudioManager()->playSE("UI_ENTER");
	mPlayerManager->equipArmer(mSelectedIndex);
}

void EquipArmerMenu::updateMenu()
{
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowArmerNum - 1) {		
		mScrollOffset++;
		refreshText();
	}
	//カーソルが上端に来たら、テキストを上にスライド
	if (mSelectedIndex < mScrollOffset) {
		mScrollOffset--;
		refreshText();
	}
}

void EquipArmerMenu::inputMenu()
{

	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex == 0) return;
		mSelectedIndex--;
		mScene.getAudioManager()->playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePositon(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex == mMaxIndex - 1) return;
		mScene.getAudioManager()->playSE("UI_MOVE1");
		mSelectedIndex++;
		if (mSelectedIndex > mScrollOffset + MaxShowArmerNum  - 1) return;
		mArrow->movePositon(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void EquipArmerMenu::refreshText()
{
	std::wstring message;
	const auto& inventory = mPlayerManager->getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = mScrollOffset; i < mScrollOffset + showArmerNum; i++) {
		message += Utility::stringToWString(inventory[i]) + L"\n";
	}
	mTextComponent->setText(message);
	mTextComponent->showText();
}

StatusMenu::StatusMenu(Game& game, TownManager& townManager, float zDepth) : Menu(game, townManager, "StatusMenu", zDepth)
{
	mMaxIndex = 2;
}

StatusMenu::~StatusMenu()
{
	mTownManager.exitStatusMenu();
}

void StatusMenu::selectedAct()
{
	mScene.getAudioManager()->playSE("UI_WINDOW_OPEN");
	switch(mSelectedIndex) {
	case 0: {
		auto weaponMenu = std::make_unique<EquipWeaponMenu>(mScene, mTownManager, 48.0f);
		mScene.addActor(std::move(weaponMenu));
		break;
	}
	case 1: {
		auto armerMenu = std::make_unique<EquipArmerMenu>(mScene, mTownManager, 48.0f);
		mScene.addActor(std::move(armerMenu));
		break;
	}
	}
}

