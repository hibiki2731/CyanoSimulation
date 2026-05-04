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

EquipWeaponMenu::EquipWeaponMenu(TownScene& scene, StatusMenu& menu,float zDepth)
	: Menu(scene, "EquipWeaponMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager()),
	mStatusMenu(menu)
{
	mMaxIndex = mPlayerManager.getPlayerData().weaponInventory.size();
	mScrollOffset = 0;

	addComponentLabel("equipIcon", "SpriteComponent");
	addComponentLabel("equipmentText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("descriptor", "TextComponent");

	applyComponentLabel();
}

void EquipWeaponMenu::applyComponentLabel()
{
	//装備中アイコン
	mEquipIcon = static_cast<SpriteComponent*>(mComponentLabels["equipIcon"].pComponent);

	//所持武器のテキストを作成
	mTextComponent = static_cast<TextComponent*>(mComponentLabels["equipmentText"].pComponent);
	if (mTextComponent) {
		std::wstring message;
		const auto& inventory = mPlayerManager.getPlayerData().weaponInventory;
		int showWeaponNum = min(inventory.size(), MaxShowWeaponNum);
		for (int i = 0; i < showWeaponNum; i++) {
			if (i == mPlayerManager.getPlayerData().equippedArmerIndex) {
				float y = mTextComponent->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + mTextComponent->getLineSpace() * i;
				mEquipIcon->setPosY(y);
			}
			message += Utility::stringToWString(mItemManager.getWeaponData(inventory[i]).name) + L"\n";
		}
		mTextComponent->setText(message);
		mTextComponent->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		mArrowMoveLength = mTextComponent->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowWeaponNum / mPlayerManager.getPlayerData().weaponInventory.size();
		if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowWeaponNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().weaponInventory.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//説明文
	mDescriptor = static_cast<TextComponent*>(mComponentLabels["descriptor"].pComponent);
	if(mDescriptor) updateDescriptor();
}

void EquipWeaponMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("SORD");
	mPlayerManager.equipWeapon(mSelectedIndex);
	mStatusMenu.applyStatus();
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


	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		updateDescriptor();
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		updateDescriptor();
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

void EquipWeaponMenu::updateDescriptor()
{
	auto playerData = mPlayerManager.getPlayerData();
	std::wstring text = Utility::stringToWString(mItemManager.getWeaponData(playerData.weaponInventory[mSelectedIndex]).description);
	mDescriptor->setText(text);
}

EquipArmerMenu::EquipArmerMenu(TownScene& scene, StatusMenu& menu, float zDepth)
	: Menu(scene, "EquipArmerMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager()),
	mStatusMenu(menu)
{
	mMaxIndex = mPlayerManager.getPlayerData().armerInventory.size();
	mScrollOffset = 0;

	addComponentLabel("equipIcon", "SpriteComponent");
	addComponentLabel("equipmentText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("descriptor", "TextComponent");

	applyComponentLabel();
}

void EquipArmerMenu::applyComponentLabel()
{
	//装備中アイコン
	mEquipIcon = static_cast<SpriteComponent*>(mComponentLabels["equipIcon"].pComponent);

	//所持防具のテキストを作成
	mTextComponent = static_cast<TextComponent*>(mComponentLabels["equipmentText"].pComponent);
	if (mTextComponent) {
		std::wstring message;
		const auto& inventory = mPlayerManager.getPlayerData().armerInventory;
		int showArmerNum = min(inventory.size(), MaxShowArmerNum);
		for (int i = 0; i < showArmerNum; i++) {
			if (i == mPlayerManager.getPlayerData().equippedArmerIndex) {
				float y = mTextComponent->getPosY() + mEquipIcon->getSpriteSize().y / 2.0f + mTextComponent->getLineSpace() * i;
				mEquipIcon->setPosY(y);
			}
			message += Utility::stringToWString(mItemManager.getArmerData(inventory[i]).name) + L"\n";
		}
		mTextComponent->setText(message);
		mTextComponent->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		mArrowMoveLength = mTextComponent->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowArmerNum / mPlayerManager.getPlayerData().armerInventory.size();
		if (mPlayerManager.getPlayerData().armerInventory.size() < MaxShowArmerNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mPlayerManager.getPlayerData().armerInventory.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//説明文
	mDescriptor = static_cast<TextComponent*>(mComponentLabels["descriptor"].pComponent);
	if(mDescriptor) updateDescriptor();
}

void EquipArmerMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("ARMER");
	mPlayerManager.equipArmer(mSelectedIndex);
	mStatusMenu.applyStatus();
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

	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		updateDescriptor();
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		mSelectedIndex++;
		updateDescriptor();
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

void EquipArmerMenu::updateDescriptor()
{
	auto playerData = mPlayerManager.getPlayerData();
	std::wstring text = Utility::stringToWString(mItemManager.getArmerData(playerData.armerInventory[mSelectedIndex]).description);
	mDescriptor->setText(text);
}

StatusMenu::StatusMenu(TownScene& scene, float zDepth)
	: Menu(scene, "StatusMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	mMaxIndex = 2;

	addComponentLabel("statusText", "TextComponent");

	applyComponentLabel();

}

void StatusMenu::applyComponentLabel()
{
	mStatusText = static_cast<TextComponent*>(mComponentLabels["statusText"].pComponent);
	if (mStatusText) applyStatus();
}

void StatusMenu::endProcessActor()
{
	Object::endProcessActor();
}

void StatusMenu::selectedAct()
{
	mScene.getGame().getAudioManager().playSE("UI_WINDOW_OPEN");	
	switch(mSelectedIndex) {
	case 0: {
		auto weaponMenu = std::make_unique<EquipWeaponMenu>(mScene, *this, 40.0f);
		mScene.addActor(std::move(weaponMenu));
		break;
	}
	case 1: {
		auto armerMenu = std::make_unique<EquipArmerMenu>(mScene, *this, 40.0f);
		mScene.addActor(std::move(armerMenu));
		break;
	}
	}
}

static int textSpaceSize = 5;
void StatusMenu::applyStatus()
{
	auto playerData = mPlayerManager.getPlayerData();
	std::wstring text;
	text = L"MAXHP " + std::to_wstring(playerData.maxHp);

	//指定した感覚になるよう空白の数を調節
	int numSpace = static_cast<int>(std::to_string(playerData.maxHp).length());
	for (int i = 0; i < textSpaceSize - numSpace; i++) text += L" ";
	text += L"POWER   " + std::to_wstring(playerData.power) + L"\n";

	text += L"MAXAP " + std::to_wstring(playerData.actionLimit);

	//指定した感覚になるよう空白の数を調節
	numSpace = static_cast<int>(std::to_string(playerData.actionLimit).length());
	for (int i = 0; i < textSpaceSize - numSpace; i++) text += L" ";
	text += L"DEFENCE " + std::to_wstring(playerData.defence) + L"\n";

	mStatusText->setText(text);
}

