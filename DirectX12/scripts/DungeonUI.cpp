#include "DungeonUI.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "DungeonScene.h"
#include "Graphic.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include "json.hpp"
#include "MyUtility.h"
#include "Player.h"
#include "AudioManager.h"
#include <fstream>
#include <string>
#include "Math.h"

DungeonUI::DungeonUI(DungeonScene& scene)
	:Object(scene, "DungeonUI"),
	mItemManager(scene.getGame().getItemManager()),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mPlayer(*scene.getPlayer()),
	mMaxMessageNum(3)
{
	addComponentLabel("hpValueText", "TextComponent");
	addComponentLabel("hpBar", "SpriteComponent");
	addComponentLabel("apValueText", "TextComponent");
	addComponentLabel("apBar", "SpriteComponent");
	addComponentLabel("originItemIcon", "SpriteComponent");
	addComponentLabel("itemSelectFrame", "SpriteComponent");
	addComponentLabel("selectItemText", "TextComponent");
	addComponentLabel("messageText", "TextComponent");
	addComponentLabel("goldText", "TextComponent");

	applyComponentLabel();
}

void DungeonUI::applyComponentLabel()
{
	//HP値
	mHPValueText = static_cast<TextComponent*>(mComponentLabels["hpValueText"].pComponent);
	if (mHPValueText) {
		int hp = mPlayer.getHP();
		int maxHp = mPlayer.getMaxHP();
		std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
		mHPValueText->setText(text);
	}

	//HPバー
	mHPBar = static_cast<SpriteComponent*>(mComponentLabels["hpBar"].pComponent);
	if (mHPBar) {
		mHPBarOriginalSize = mHPBar->getSpriteSize();
		XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(mPlayer.getHP()) / static_cast<float>(mPlayer.getMaxHP()), mHPBarOriginalSize.y);
		if (hpBarSize.y < 10.0f) hpBarSize.y = 10.0f; //HPバーの最小サイズ
		mHPBar->setSpriteSize(hpBarSize);
	}

	//AP値
	mAPValueText = static_cast<TextComponent*>(mComponentLabels["apValueText"].pComponent);
	if (mAPValueText) {
		std::wstring text = std::to_wstring(mPlayer.getAP()) + L"/" + std::to_wstring(mPlayer.getMaxAP()) + L"\n";
		mAPValueText->setText(text);
	}

	//APバー
	mAPBar = static_cast<SpriteComponent*>(mComponentLabels["apBar"].pComponent);
	if (mAPBar) {
		mAPBarOriginalSize = mAPBar->getSpriteSize();
	}

	//アイテムアイコン
	auto originItemIcon = static_cast<SpriteComponent*>(mComponentLabels["originItemIcon"].pComponent);
	if (originItemIcon && mItemIcons.size() == 0) {
		//アイテムアイコン配列に追加
		mItemIcons.push_back(originItemIcon);
		mFrameOriginPos = originItemIcon->getPosition();

		for (int i = 1; i < mPlayer.getStorageSize(); i++) {
			auto itemIcon = std::make_unique<SpriteComponent>(*this);
			XMFLOAT3 position = mItemIcons[0]->getPosition();
			position.x += i * (mItemIcons[0]->getSpriteSize().x + 10.0f); //アイテムアイコンの間隔を10.0fとする
			itemIcon->setPosition(position);
			itemIcon->setSpriteSize(mItemIcons[0]->getSpriteSize());
			itemIcon->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
			mItemIcons.push_back(itemIcon.get());
			addComponent(std::move(itemIcon));
		}

		//アイコンの画像を反映
		updateItemIcon();

	}

	//選択アイテム枠
	mItemSelectFrame = static_cast<SpriteComponent*>(mComponentLabels["itemSelectFrame"].pComponent);
	//選択アイテムテキスト
	mSelectItemText = static_cast<TextComponent*>(mComponentLabels["selectItemText"].pComponent);
	if (mSelectItemText) {
		std::wstring selectItemTextStr = Utility::stringToWString(mItemManager.getItemData(mPlayer.getSelectItemID()).name) + L"\n";
		mSelectItemText->setText(selectItemTextStr);
	}

	//メッセージテキスト
	mMessageText = static_cast<TextComponent*>(mComponentLabels["messageText"].pComponent);
	if (mMessageText) {
		static std::wstring firstMessage = L"";
		mMessageText->setText(firstMessage);
	}

	//所持金の表示
	mGoldText = static_cast<TextComponent*>(mComponentLabels["goldText"].pComponent);
	if (mGoldText) updateGold();
}

void DungeonUI::updateHP()
{
	//現在のHPを取得
	int hp =mPlayer.getHP();
	int maxHp = mPlayer.getMaxHP();
	//HP値のテキストを更新
	std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
	mHPValueText->setText(text);
	//HPバーのサイズを更新
	XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(hp) / static_cast<float>(maxHp), mHPBarOriginalSize.y);
	if (hpBarSize.x < 10.0f) hpBarSize.x = 10.0f; //HPバーの最小サイズ
	if(mHPBar) mHPBar->setSpriteSize(hpBarSize);
}

void DungeonUI::updateAP()
{
	//現在のAPを取得
	int ap = mPlayer.getAP();
	int maxAp = mPlayer.getMaxAP();
	//AP値のテキストを更新
	std::wstring text = std::to_wstring(ap) + L"/" + std::to_wstring(maxAp) + L"\n";
	mAPValueText->setText(text);
	//APバーのサイズを更新
	XMFLOAT2 apBarSize = XMFLOAT2(mAPBarOriginalSize.x * static_cast<float>(ap) / static_cast<float>(maxAp), mAPBarOriginalSize.y);
	if (apBarSize.x < 10.0f) apBarSize.x = 10.0f; //HPバーの最小サイズ
	if(mAPBar) mAPBar->setSpriteSize(apBarSize);
}

void DungeonUI::updateItemIcon()
{
	//アイテムアイコンを更新
	if (mItemIcons.size() != 0) {
		//アイテムの更新
		for (int i = 0; i < mPlayer.getStorageSize(); i++) {
			if (i < mPlayerManager.getInventory().size())
				mItemIcons[i]->create(mItemManager.getItemData(mPlayerManager.getInventoryItem(i)).iconFilePath);
			else
				mItemIcons[i]->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
		}
	}
}

void DungeonUI::updateItemFrame()
{
	//選択アイテムの枠を更新
	int selectIndex = mPlayer.getSelectItemIndex();
	mItemSelectFrame->setPosition(mFrameOriginPos + XMFLOAT3((mItemSelectFrame->getSpriteSize().x + 10.0f) * selectIndex, 0.0f, 0.0f));

	//選択アイテムの名前を更新
	std::wstring selectItemText = Utility::stringToWString(mItemManager.getItemData(mPlayer.getSelectItemID()).name) + L"\n";
	mSelectItemText->setText(selectItemText);
}

void DungeonUI::pushMessage(const std::string& message)
{
	if (mMessages.size() >= mMaxMessageNum) mMessages.pop_front();
	mMessages.push_back(message);

	std::string text;
	for (int i = mMessages.size() - 1; i >= 0; i--) {
		text += mMessages[i] + "\n";
	}
	mMessageText->setText(Utility::stringToWString(text));
}

void DungeonUI::updateGold()
{
	if (!mGoldText) return;

	int goldValue = mItemManager.getResourceNum("GOLD");
	std::wstring text = std::to_wstring(goldValue) + L" G\n";
	mGoldText->setText(text);
}
