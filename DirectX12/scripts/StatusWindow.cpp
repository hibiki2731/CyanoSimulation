#include "StatusWindow.h"
#include "TownScene.h"
#include "Game.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include "GUIDebugger.h"


StatusWindow::StatusWindow(TownScene& scene, float zDepth)
	: Actor(scene),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	std::string structName;

	//プレイヤーのステータスUI
	//キャンバス
	structName = "StatusCanvas";
	auto canvas = std::make_unique<SpriteComponent>(*this, zDepth);
	canvas->loadFileAndCreate(structName);
#ifdef _DEBUG
	canvas->activateControll(structName);
#endif
	addComponent(std::move(canvas));

	//プレイヤーデータの取得
	auto playerData = mPlayerManager.getPlayerData();

	{
		std::wstring text = L"HP\n";
		//HPテキスト
		structName = "HPText";
		auto hpText = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
		hpText->loadFileAndCreate(structName);
#ifdef _DEBUG
		hpText->activateControll(structName);
#endif
		addComponent(std::move(hpText));

		//HP値のテキスト
		structName = "HPValueText";
		auto hpValueText = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
		hpValueText->loadFileAndCreate(structName);
		int hp = playerData.hp;
		int maxHp = playerData.maxHp;
		text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
		hpValueText->setText(text);
		mHPValueText = hpValueText.get();
#ifdef _DEBUG
		hpValueText->activateControll(structName);
#endif
		addComponent(std::move(hpValueText));

		//HPバー
		structName = "HPBar";
		auto hpBar = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
		hpBar->loadFileAndCreate(structName);
		mHPBarOriginalSize = hpBar->getSpriteSize();
		XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(hp) / static_cast<float>(maxHp), mHPBarOriginalSize.y);
		if (hpBarSize.y < 10.0f) hpBarSize.y = 10.0f; //HPバーの最小サイズ
		hpBar->setSpriteSize(hpBarSize);
		mHPBar = hpBar.get();
#ifdef _DEBUG
		hpBar->activateControll(structName);
#endif
		addComponent(std::move(hpBar));

		//HPバーの背景
		structName = "HPBarBack";
		auto hpBarBack = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
		hpBarBack->loadFileAndCreate(structName);
#ifdef _DEBUG
		hpBarBack->activateControll(structName);
#endif
		addComponent(std::move(hpBarBack));
	}

	{
		//APテキスト
		structName = "APText";
		auto apText = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
		apText->loadFileAndCreate(structName);
		std::wstring text = L"AP\n";
#ifdef _DEBUG
		apText->activateControll(structName);
#endif
		addComponent(std::move(apText));

		//AP値のテキスト
		structName = "APValueText";
		auto apValueText = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
		apValueText->loadFileAndCreate(structName);
		text = std::to_wstring(playerData.actionLimit) + L"/" + std::to_wstring(playerData.actionLimit) + L"\n";
		apValueText->setText(text);
		mAPValueText = apValueText.get();
#ifdef _DEBUG
		apValueText->activateControll(structName);
#endif

		addComponent(std::move(apValueText));

		//APバー
		structName = "APBar";
		auto apBar = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
		apBar->loadFileAndCreate(structName);
#ifdef _DEBUG
		apBar->activateControll(structName);
#endif 
		addComponent(std::move(apBar));

		//APバーの背景
		structName = "APBarBack";
		auto apBarBack = std::make_unique<SpriteComponent>(*this, zDepth - 0.5f);
		apBarBack->loadFileAndCreate(structName);
#ifdef _DEBUG
		apBarBack->activateControll(structName);
#endif
		addComponent(std::move(apBarBack));
	}

	//アイテムアイコン
	structName = "ItemIconOrigin";
	auto originItemIcon = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	originItemIcon->loadFileAndCreate(structName);
	if (playerData.inventory.size() == 0)
		originItemIcon->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
	else
		originItemIcon->create(mItemManager.getItemData(playerData.inventory[0]).iconFilePath);
	mItemIcons.push_back(originItemIcon.get());
#ifdef _DEBUG
	originItemIcon->activateControll(structName);
#endif
	for (int i = 1; i < playerData.storageSize; i++) {
		auto itemIcon = std::make_unique<SpriteComponent>(*this);
		XMFLOAT3 position = originItemIcon->getPosition();
		position.x += i * (originItemIcon->getSpriteSize().x + 10.0f); //アイテムアイコンの間隔を10.0fとする
		itemIcon->setPosition(position);
		itemIcon->setSpriteSize(originItemIcon->getSpriteSize());
		if (i > static_cast<int>(playerData.inventory.size()) - 1)
			itemIcon->create("assets/picture/UI2/PNG/Default/panel_grey_bolts.png");
		else 
			itemIcon->create(mItemManager.getItemData(playerData.inventory[i]).iconFilePath);
		mItemIcons.push_back(itemIcon.get());
		addComponent(std::move(itemIcon));
	}
	addComponent(std::move(originItemIcon));
	
}

void StatusWindow::updateStatus()
{
	//プレイヤーデータの取得
	auto playerData = mPlayerManager.getPlayerData();

	//HP値のテキストを更新
	int hp = playerData.hp;
	int maxHp = playerData.maxHp;
	std::wstring text = std::to_wstring(hp) + L"/" + std::to_wstring(maxHp) + L"\n";
	mHPValueText->setText(text);

	//HPバーのサイズを更新
	XMFLOAT2 hpBarSize = XMFLOAT2(mHPBarOriginalSize.x * static_cast<float>(hp) / static_cast<float>(maxHp), mHPBarOriginalSize.y);
	if (hpBarSize.x < 10.0f) hpBarSize.x = 10.0f; //HPバーの最小サイズ
	mHPBar->setSpriteSize(hpBarSize);

	//AP値のテキストを更新
	int ap = playerData.actionLimit;
	std::wstring apText = std::to_wstring(ap) + L"/" + std::to_wstring(ap) + L"\n";
	mAPValueText->setText(apText);

	//アイテムアイコンを更新
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
