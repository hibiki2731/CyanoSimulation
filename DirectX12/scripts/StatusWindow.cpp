#include "StatusWindow.h"
#include "TownScene.h"
#include "Game.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include "GUIDebugger.h"


StatusWindow::StatusWindow(TownScene& scene, float zDepth)
	: Object(scene, "statusWindow"),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{

	addComponentLabel("hpValueText", "TextComponent");
	addComponentLabel("hpBar", "SpriteComponent");
	addComponentLabel("apValueText", "TextComponent");
	addComponentLabel("originItemIcon", "SpriteComponent");
	addComponentLabel("goldText", "TextComponent");
	
	applyComponentLabel();
}

void StatusWindow::applyComponentLabel()
{
	//プレイヤーデータの取得
	auto playerData = mPlayerManager.getPlayerData();

	//HP値のテキスト
	mHPValueText = static_cast<TextComponent*>(mComponentLabels["hpValueText"].pComponent);
	if (mHPValueText) {
		int hp = playerData.hp;
		int maxHp = playerData.maxHp;
		std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
		mHPValueText->setText(text);
	}

	//HPバー
	mHPBar = static_cast<SpriteComponent*>(mComponentLabels["hpBar"].pComponent);
	if (mHPBar) {
		mHPBarOriginalSize = mHPBar->getSpriteSize();
		XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(playerData.hp) / static_cast<float>(playerData.maxHp), mHPBarOriginalSize.y);
		if (hpBarSize.y < 10.0f) hpBarSize.y = 10.0f; //HPバーの最小サイズ
		mHPBar->setSpriteSize(hpBarSize);
	}

		//AP値のテキスト
	mAPValueText = static_cast<TextComponent*>(mComponentLabels["apValueText"].pComponent);
	if (mAPValueText) {
		std::wstring text = std::to_wstring(playerData.actionLimit) + L"/" + std::to_wstring(playerData.actionLimit) + L"\n";
		mAPValueText->setText(text);
	}

	//アイテムアイコン
	auto originItemIcon = static_cast<SpriteComponent*>(mComponentLabels["originItemIcon"].pComponent);
	if (originItemIcon && mItemIcons.size() == 0) {
		
		//アイテムアイコンの画像を判別
		if (playerData.inventory.size() == 0)
			originItemIcon->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
		else
			originItemIcon->create(mItemManager.getItemData(playerData.inventory[0]).iconFilePath);

		//アイテムアイコン配列に追加
		mItemIcons.push_back(originItemIcon);
		updateStatus();

	}

	//所持金
	mGoldText = static_cast<TextComponent*>(mComponentLabels["goldText"].pComponent);
	if (mGoldText) {
		int goldValue = mItemManager.getResourceNum("GOLD");
		std::wstring text = std::to_wstring(goldValue) + L" G\n";
		mGoldText->setText(text);
	}
}

void StatusWindow::updateStatus()
{
	//プレイヤーデータの取得
	auto playerData = mPlayerManager.getPlayerData();

	//HP値のテキストを更新
	int hp = playerData.hp;
	int maxHp = playerData.maxHp;
	std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
	if(mHPValueText) mHPValueText->setText(text);

	//HPバーのサイズを更新
	XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(hp) / static_cast<float>(maxHp), mHPBarOriginalSize.y);
	if (hpBarSize.x < 10.0f) hpBarSize.x = 10.0f; //HPバーの最小サイズ
	if(mHPBar) mHPBar->setSpriteSize(hpBarSize);

	//AP値のテキストを更新
	int ap = playerData.actionLimit;
	std::wstring apText = std::to_wstring(ap) + L"/" + std::to_wstring(ap) + L"\n";
	if(mAPValueText) mAPValueText->setText(apText);

	//アイテムアイコンを更新
	if (mItemIcons.size() != 0) {
		//ストレージサイズが変更されていた場合
		if (mItemIcons.size() < playerData.storageSize) {
			for (int i = mItemIcons.size(); i < playerData.storageSize; i++) {
				auto itemIcon = std::make_unique<SpriteComponent>(*this);
				XMFLOAT3 position = mItemIcons[0]->getPosition();
				position.x += i * (mItemIcons[0]->getSpriteSize().x + 10.0f); //アイテムアイコンの間隔を10.0fとする
				itemIcon->setPosition(position);
				itemIcon->setSpriteSize(mItemIcons[0]->getSpriteSize());
				itemIcon->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
				mItemIcons.push_back(itemIcon.get());
				addComponent(std::move(itemIcon));
			}
		}

		//アイテムの更新
		for (int i = 0; i < playerData.storageSize; i++) {
			if (i < playerData.inventory.size())
				mItemIcons[i]->create(mItemManager.getItemData(playerData.inventory[i]).iconFilePath);
			else
				mItemIcons[i]->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
		}
	}

	//所持金の更新
	int goldValue = mItemManager.getResourceNum("GOLD");
	text = std::to_wstring(goldValue) + L" G\n";
	if(mGoldText) mGoldText->setText(text);


}
