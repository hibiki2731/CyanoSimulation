#include "ShopMenu.h"
#include "TownScene.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include "AudioManager.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "MyUtility.h"
#include "input.h"

ShopMenu::ShopMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ShopMenu", zDepth),
	mItemManager(scene.getGame().getItemManager()),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	prepareSaleItem();
	mScrollOffset = 0;

	static float fontSize =40.0f;
	static float lineSpace = 8.0f;

	//購入可能な武器と防具のテキストを作成
	std::wstring shopText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(fontSize);
	textComponent->setLineSpace(lineSpace);
	textComponent->setBaseLine(mPosition.x + 60.0f, mPosition.y + 75.0f);
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& saleItemID : mSaleItem) {
		const auto& itemData = mItemManager.getItemData(saleItemID);
		shopText += Utility::stringToWString(itemData.name) + L"\n";
	}
	if (shopText.size() == 0) shopText = L"なし\n";
	textComponent->setText(shopText);
	textComponent->showText();
	mShopText = textComponent.get();
	addComponent(std::move(textComponent));

	//インジケーターの移動距離を設定
	mArrowMoveLength = fontSize + lineSpace;

	//スクロールバー
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + MaxShowItemNum * 48.0f - 8.0f, zDepth - 0.5f));
	downArrow->setBordarSize(0.0f);
	downArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	downArrow->setRotation(XM_PI);
	addComponent(std::move(downArrow));

	//上矢印
	auto upArrow = std::make_unique<SpriteComponent>(*this);
	upArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	upArrow->setPosition(XMFLOAT3(80.0f, 175.0f, zDepth - 0.5f));
	upArrow->setBordarSize(0.0f);
	upArrow->setSpriteSize(XMFLOAT2(25.0f, 25.0f));
	addComponent(std::move(upArrow));

	//スクロールバー
	auto scrollBar = std::make_unique<SpriteComponent>(*this);
	scrollBar->create("assets/picture/UI2/PNG/Default/scrollbar_future_grey.png");
	scrollBar->setPosition(XMFLOAT3(80.0f, 175.0f + 30.0f, zDepth - 0.5f));
	scrollBar->setBordarSize(10.0f);
	float arrowDistance = 48.0f * MaxShowItemNum - 38.0f;
	float height = arrowDistance * MaxShowItemNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowItemNum) height = arrowDistance;
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));
}

void ShopMenu::selectedAct()
{
	buyItem(mSelectedIndex);
}

void ShopMenu::updateMenu()
{
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowItemNum - 1) {		
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

void ShopMenu::inputMenu()
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
		if (mSelectedIndex > mScrollOffset + MaxShowItemNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void ShopMenu::prepareSaleItem()
{
	for (auto& data : mItemManager.getItemData()) {
		mSaleItem.emplace_back(data.second.id);
	}

	mMaxIndex = mSaleItem.size();
}

void ShopMenu::refreshText()
{
	//テキストの更新
	std::wstring message;
	int showItemNum = min(mSaleItem.size(), MaxShowItemNum);
	for (int i = mScrollOffset; i < mScrollOffset + showItemNum; i++) {
		const auto& itemData = mItemManager.getWeaponData(mSaleItem[i]);
		message += Utility::stringToWString(itemData.name) + L"\n";
	}
	mShopText->setText(message);
	mShopText->showText();
}

void ShopMenu::buyItem(int index) {
	//リソースを消費
	const auto& itemData = mItemManager.getItemData(mSaleItem[index]);
	for (int i = 0; i < itemData.costResourceID.size(); i++) {
		int possessedResource = mItemManager.getResourceNum(itemData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (itemData.price[i] > possessedResource) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}

		//所持リソースを消費リソース分減らす
		mItemManager.subResource(itemData.costResourceID[i], itemData.price[i]);
	}

	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	//インベントリにアイテムを追加
	mPlayerManager.addInventory(itemData.id);
}


