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
#include "ItemManager.h"

EquipWeaponMenu::EquipWeaponMenu(TownScene& scene, float zDepth)
	: Menu(scene, "EquipWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	mMaxIndex = mPlayerManager.getPlayerData().weaponInventory.size();
	mScrollOffset = 0;

	//ファイルの読み込み
	std::string structName;

	//装備中アイコン
	structName = "EquipWeaponMenuEquipIcon";
	auto equipIcon = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	equipIcon->loadFileAndCreate(structName);
	equipIcon->setPosY(-100.0f); //初期位置はテキストの外にしておく
	mEquipIcon = equipIcon.get();
#ifdef _DEBUG
	equipIcon->activateControll(structName);
#endif
	addComponent(std::move(equipIcon));

	//所持武器のテキストを作成
	structName = "EquipWeaponMenuScrollText";
	auto weaponText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	weaponText->loadFileAndCreate(structName);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().weaponInventory;
	int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
	for (int i = 0; i < showWeaponNum; i++) {
		if (mPlayerManager.getPlayerData().equippedWeaponIndex == i) {
			float y = weaponText->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + weaponText->getLineSpace() * i;
			mEquipIcon->setPosY(y);
		}
		message += Utility::stringToWString(mItemManager.getWeaponData(inventory[i]).name) + L"\n";
	}
	weaponText->setText(message);
	weaponText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	weaponText->activateControll(structName);
#endif

	mArrowMoveLength = weaponText->getLineSpace();
	mTextComponent = weaponText.get();
	addComponent(std::move(weaponText));


	//スクロールインジケーター
	//下矢印
	structName = "EquipWeaponMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	addComponent(std::move(downArrow));

	//上矢印
	structName = "EquipWeaponMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "EquipWeaponMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().y;
	float height = maxHeight * MaxShowWeaponNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowWeaponNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(scrollBar->getSpriteSize().x, height));
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
	mEquipIcon->setPosY(-100.0f); //一旦装備中アイコンをテキストの外に出す
	for (int i = mScrollOffset; i < mScrollOffset + showWeaponNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedWeaponIndex) {
			float y = mTextComponent->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + mTextComponent->getLineSpace() * (i - mScrollOffset);
			mEquipIcon->setPosY(y);
		}
		message += Utility::stringToWString(mItemManager.getWeaponData(inventory[i]).name) + L"\n";
	}
	mTextComponent->setText(message);

}

EquipArmerMenu::EquipArmerMenu(TownScene& scene, float zDepth)
	: Menu(scene, "EquipArmerMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	mMaxIndex = mPlayerManager.getPlayerData().armerInventory.size();
	mScrollOffset = 0;

	std::string structName;

	//装備中アイコン
	structName = "EquipWeaponMenuEquipIcon";
	auto equipIcon = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	equipIcon->loadFileAndCreate(structName);
	equipIcon->setPosY(-100.0f); //初期位置はテキストの外にしておく
	mEquipIcon = equipIcon.get();
#ifdef _DEBUG
	equipIcon->activateControll(structName);
#endif
	addComponent(std::move(equipIcon));

	//所持防具のテキストを作成
	structName = "EquipArmerMenuScrollText";
	auto armerText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	armerText->loadFileAndCreate(structName);
	std::wstring message;
	const auto& inventory = mPlayerManager.getPlayerData().armerInventory;
	int showArmerNum = min(inventory.size(), MaxShowArmerNum);
	for (int i = 0; i < showArmerNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedArmerIndex) {
			float y = armerText->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + armerText->getLineSpace() * i;
			mEquipIcon->setPosY(y);
		}
		message += Utility::stringToWString(mItemManager.getArmerData(inventory[i]).name) + L"\n";
	}
	armerText->setText(message);
	armerText->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
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
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "EquipArmerMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().y;
	float height = maxHeight * MaxShowArmerNum / mPlayerManager.getPlayerData().armerInventory.size();
	if (mPlayerManager.getPlayerData().armerInventory.size() < MaxShowArmerNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().armerInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(scrollBar->getSpriteSize().x, height));
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
	mEquipIcon->setPosY(-100.0f); //一旦装備中アイコンをテキストの外に出す
	for (int i = mScrollOffset; i < mScrollOffset + showArmerNum; i++) {
		if (i == mPlayerManager.getPlayerData().equippedArmerIndex) {
			float y = mTextComponent->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + mTextComponent->getLineSpace() * (i - mScrollOffset);
			mEquipIcon->setPosY(y);
		}
		message += Utility::stringToWString(mItemManager.getArmerData(inventory[i]).name) + L"\n";
	}
	mTextComponent->setText(message);
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

