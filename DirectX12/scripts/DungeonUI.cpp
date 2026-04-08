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
#include <fstream>
#include <string>

constexpr float CanvasWidth = Graphic::ClientWidth * 0.3f;
constexpr float CanvasHeight = Graphic::ClientHeight * 0.2;
constexpr float CanvasZ = 50.0f;
XMFLOAT2 DungeonUI::ItemIconOriginPos = { 10.0f, 52.0f };
XMFLOAT2 DungeonUI::ItemIconSize = { 60.0f, 60.0f };

DungeonUI::DungeonUI(DungeonScene& scene)
	:Actor(scene),
	mItemManager(scene.getGame().getItemManager()),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mPlayer(*scene.getPlayer())
{
	nlohmann::json uiData;
	std::fstream file("assets\\data\\dungeonUIData.json");
	file >> uiData;

	//キャンバス
	auto canvas = std::make_unique<SpriteComponent>(*this);
	canvas->create("assets/picture/UI2/PNG/Default/panel_grey.png");
	std::vector<float> canvasPos = uiData["canvas"]["pos"].get<std::vector<float>>();
	canvas->setPosition(XMFLOAT3(canvasPos[0], canvasPos[1], CanvasZ));
	canvas->setSpriteSize(XMFLOAT2(CanvasWidth, CanvasHeight));
	canvas->setBordarSize(24.0f);
	mCanvas = canvas.get();
	addComponent(std::move(canvas));

	{
		std::wstring text = L"HP\n";
		//HPテキスト
		auto hpText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		float hpTextFontSize = uiData["hp"]["textFontSize"].get<float>();
		hpText->setFontSize(hpTextFontSize);
		std::vector<float> hpTextPosition = uiData["hp"]["textPosition"].get<std::vector<float>>();
		hpText->setBaseLine(hpTextPosition[0], hpTextPosition[1]);
		hpText->setText(text);
		hpText->showText();
		addComponent(std::move(hpText));

		//HP値のテキスト
		auto hpValueText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		hpValueText->setFontSize(hpTextFontSize);
		hpValueText->setBaseLine(hpTextPosition[0] + uiData["hp"]["textSpace"].get<float>(), hpTextPosition[1]);
		int hp = mPlayer.getHP();
		int maxHp = mPlayer.getMaxHP();
		text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
		hpValueText->setText(text);
		hpValueText->showText();
		mHPValueText = hpValueText.get();
		addComponent(std::move(hpValueText));

		//HPバー
		auto hpBar = std::make_unique<SpriteComponent>(*this);
		hpBar->create(uiData["hp"]["barTexturePath"].get<std::string>());
		mHPBarOriginalSize = XMFLOAT2(hpTextFontSize * 0.7f, CanvasWidth * 0.5f);
		XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x, mHPBarOriginalSize.y * hp / maxHp);
		if (hpBarSize.y < 10.0f) hpBarSize.y = 10.0f; //HPバーの最小サイズ
		hpBar->setSpriteSize(hpBarSize);
		auto hpBarOffsetPos = uiData["hp"]["barOffsetPos"].get<std::vector<float>>();
		mHPBarOffsetPos = XMFLOAT2(CanvasWidth * 0.4 + hpBarOffsetPos[0], hpTextPosition[1] + hpBarOffsetPos[1]);
		hpBar->setPosition(XMFLOAT3(mHPBarOffsetPos.x + (hpBarSize.y - hpBarSize.x) * 0.5f, mHPBarOffsetPos.y - (hpBarSize.y - hpBarSize.x) * 0.5f, CanvasZ - 1.0f));
		hpBar->setBordarSize(5.0f);
		hpBar->setRotation(XM_PIDIV2);
		mHPBar = hpBar.get();
		addComponent(std::move(hpBar));

		//HPバーの背景
		auto hpBarBack = std::make_unique<SpriteComponent>(*this);
		hpBarBack->create(uiData["hp"]["barBackTexturePath"].get<std::string>());
		hpBarBack->setSpriteSize(mHPBarOriginalSize);
		hpBarBack->setPosition(XMFLOAT3(CanvasWidth * 0.4 + hpBarOffsetPos[0] + (mHPBarOriginalSize.y - mHPBarOriginalSize.x) * 0.5f, hpTextPosition[1] + hpBarOffsetPos[1] - (mHPBarOriginalSize.y - mHPBarOriginalSize.x) * 0.5f, CanvasZ - 0.5f));
		hpBarBack->setBordarSize(5.0f);
		hpBarBack->setRotation(XM_PIDIV2);
		addComponent(std::move(hpBarBack));
	}

	{
		std::wstring text = L"AP\n";
		//APテキスト
		auto apText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		float apTextFontSize = uiData["ap"]["textFontSize"].get<float>();
		apText->setFontSize(apTextFontSize);
		std::vector<float> apTextPosition = uiData["ap"]["textPosition"].get<std::vector<float>>();
		apText->setBaseLine(apTextPosition[0], apTextPosition[1]);
		apText->setText(text);
		apText->showText();
		addComponent(std::move(apText));

		//AP値のテキスト
		auto apValueText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		apValueText->setFontSize(apTextFontSize);
		apValueText->setBaseLine(apTextPosition[0] + uiData["ap"]["textSpace"].get<float>(), apTextPosition[1]);
		text = std::to_wstring(mPlayer.getAP()) + L"/" + std::to_wstring(mPlayer.getMaxAP()) + L"\n";
		apValueText->setText(text);
		apValueText->showText();
		mAPValueText = apValueText.get();
		addComponent(std::move(apValueText));

		//APバー
		auto apBar = std::make_unique<SpriteComponent>(*this);
		apBar->create(uiData["ap"]["barTexturePath"].get<std::string>());
		 mAPBarOriginalSize = XMFLOAT2(apTextFontSize * 0.7f, CanvasWidth * 0.5f);
		apBar->setSpriteSize(mAPBarOriginalSize);
		auto apBarOffsetPos = uiData["ap"]["barOffsetPos"].get<std::vector<float>>();
		mAPBarOffsetPos = XMFLOAT2(CanvasWidth * 0.4 + apBarOffsetPos[0], apTextPosition[1] + apBarOffsetPos[1]);
		apBar->setPosition(XMFLOAT3(mAPBarOffsetPos.x + (mAPBarOriginalSize.y - mAPBarOriginalSize.x) * 0.5f, mAPBarOffsetPos.y - (mAPBarOriginalSize.y - mAPBarOriginalSize.x) * 0.5f, CanvasZ - 1.0f));
		apBar->setBordarSize(5.0f);
		apBar->setRotation(XM_PIDIV2);
		mAPBar = apBar.get();
		addComponent(std::move(apBar));

		//APバーの背景
		auto apBarBack = std::make_unique<SpriteComponent>(*this);
		apBarBack->create(uiData["ap"]["barBackTexturePath"].get<std::string>());
		apBarBack->setSpriteSize(mAPBarOriginalSize);
		apBarBack->setPosition(XMFLOAT3(CanvasWidth * 0.4 + apBarOffsetPos[0] + (mAPBarOriginalSize.y - mAPBarOriginalSize.x) * 0.5f, apTextPosition[1] + apBarOffsetPos[1] - (mAPBarOriginalSize.y - mAPBarOriginalSize.x) * 0.5f, CanvasZ - 0.5f));
		apBarBack->setBordarSize(5.0f);
		apBarBack->setRotation(XM_PIDIV2);
		addComponent(std::move(apBarBack));
	}

	//アイテムアイコン
	for (int i = 0; i < scene.getPlayer()->getStorageSize(); i++) {
		auto itemIcon = std::make_unique<SpriteComponent>(*this);
		itemIcon->setPosition(XMFLOAT3(ItemIconOriginPos.x + (ItemIconSize.x + 10.0f) * i , ItemIconOriginPos.y, CanvasZ - 1.0f));
		itemIcon->setSpriteSize(ItemIconSize);
		itemIcon->create(uiData["itemIcon"][mItemManager.getItemData(mPlayerManager.getInventoryItem(i)).category].get<std::string>());
		mItemIcons.push_back(itemIcon.get());
		addComponent(std::move(itemIcon));
	}

	{
		//選択中アイテムの枠
		auto itemSelectFrame = std::make_unique<SpriteComponent>(*this);
		itemSelectFrame->create("assets/picture/SelectFrame.png");
		itemSelectFrame->setSpriteSize(ItemIconSize);
		itemSelectFrame->setPosition(XMFLOAT3(ItemIconOriginPos.x, ItemIconOriginPos.y, CanvasZ - 2.0f));
		mItemSelectFrame = itemSelectFrame.get();
		addComponent(std::move(itemSelectFrame));

		//選択アイテムの名前
		auto selectItemText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		selectItemText->setFontSize(uiData["selectItemText"]["fontSize"].get<float>());
		auto selectItemTextPosition = uiData["selectItemText"]["position"].get<std::vector<float>>();
		selectItemText->setBaseLine(selectItemTextPosition[0], selectItemTextPosition[1]);
		std::wstring selectItemTextStr = Utility::stringToWString(mItemManager.getItemData(mPlayer.getSelectItemID()).name) + L"\n";
		selectItemText->setText(selectItemTextStr);
		selectItemText->showText();
		mSelectItemText = selectItemText.get();
		addComponent(std::move(selectItemText));
	}

	{
		//所持金
		//auto moneyText = std::make_unique<TextComponent>(*this, CanvasZ - 1.0f);
		//moneyText->setFontSize
	}

}

void DungeonUI::updateHP()
{
	//現在のHPを取得
	int hp =mPlayer.getHP();
	int maxHp = mPlayer.getMaxHP();
	//HP値のテキストを更新
	std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
	mHPValueText->setText(text);
	mHPValueText->showText();
	//HPバーのサイズを更新
	XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x, mHPBarOriginalSize.y * static_cast<float>(hp) / static_cast<float>(maxHp));
	if (hpBarSize.y < 10.0f) hpBarSize.y = 10.0f; //HPバーの最小サイズ
	mHPBar->setSpriteSize(hpBarSize);
	mHPBar->setPosition(XMFLOAT3(mHPBarOffsetPos.x + (hpBarSize.y - hpBarSize.x) * 0.5f, mHPBarOffsetPos.y - (hpBarSize.y - hpBarSize.x) * 0.5f, CanvasZ - 1.0f));
}

void DungeonUI::updateAP()
{
	//現在のAPを取得
	int ap = mPlayer.getAP();
	int maxAp = mPlayer.getMaxAP();
	//AP値のテキストを更新
	std::wstring text = std::to_wstring(ap) + L"/" + std::to_wstring(maxAp) + L"\n";
	mAPValueText->setText(text);
	mAPValueText->showText();
	//APバーのサイズを更新
	XMFLOAT2 apBarSize = XMFLOAT2(mAPBarOriginalSize.x, mAPBarOriginalSize.y * static_cast<float>(ap) / static_cast<float>(maxAp));
	if (apBarSize.y < 10.0f) apBarSize.y = 10.0f; //HPバーの最小サイズ
	mAPBar->setSpriteSize(apBarSize);
	mAPBar->setPosition(XMFLOAT3(mAPBarOffsetPos.x + (apBarSize.y - apBarSize.x) * 0.5f, mAPBarOffsetPos.y - (apBarSize.y - apBarSize.x) * 0.5f, CanvasZ - 1.0f));
}

void DungeonUI::updateItemIcon()
{
	nlohmann::json uiData;
	std::fstream file("assets\\data\\dungeonUIData.json");
	file >> uiData;

	//アイテムアイコンを空にする
	for (auto itemIcon : mItemIcons) {
		removeComponent(itemIcon);
	}
	mItemIcons.clear();

	for (int i = 0; i < mPlayer.getStorageSize(); i++) {
		auto itemIcon = std::make_unique<SpriteComponent>(*this);
		itemIcon->setPosition(XMFLOAT3(ItemIconOriginPos.x + (ItemIconSize.x + 10.0f) * i , ItemIconOriginPos.y, CanvasZ - 1.0f));
		itemIcon->setSpriteSize(ItemIconSize);
		itemIcon->create(uiData["itemIcon"].value(mItemManager.getItemData(mPlayerManager.getInventoryItem(i)).category, "assets/picture/UI2/PNG/Default/panel_grey_bolts.png"));
		mItemIcons.push_back(itemIcon.get());
		addComponent(std::move(itemIcon));
	}


}

void DungeonUI::updateItemFrame()
{
	//選択アイテムの枠を更新
	int selectIndex = mPlayer.getSelectItemIndex();
	mItemSelectFrame->setPosition(XMFLOAT3(ItemIconOriginPos.x + (ItemIconSize.x + 10.0f) * selectIndex, ItemIconOriginPos.y, CanvasZ - 2.0f));

	//選択アイテムの名前を更新
	std::wstring selectItemText = Utility::stringToWString(mItemManager.getItemData(mPlayer.getSelectItemID()).name) + L"\n";
	mSelectItemText->setText(selectItemText);
	mSelectItemText->showText();
}
