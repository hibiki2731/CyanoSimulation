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
#include "json.hpp"
#include <fstream>

ShopMenu::ShopMenu(TownScene& scene, float zDepth)
	: Menu(scene, "ShopMenu", zDepth),
	mItemManager(scene.getGame().getItemManager()),
	mPlayerManager(scene.getGame().getPlayerManager())
{
	prepareSaleItem();
	mScrollOffset = 0;

	addComponentLabel("shopText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("resourceText", "TextComponent");
	addComponentLabel("itemEffectText", "TextComponent");
	addComponentLabel("costText", "TextComponent");

	applyComponentLabel();

}

void ShopMenu::applyComponentLabel()
{
	mShopText = static_cast<TextComponent*>(mComponentLabels["shopText"].pComponent);
	if(mShopText){
		//購入可能な武器と防具のテキストを作成
		std::wstring shopText = L"";
		for (const auto& saleItemID : mSaleItem) {
			const auto& itemData = mItemManager.getItemData(saleItemID);
			shopText += Utility::stringToWString(itemData.name) + L"\n";
		}
		if (shopText.size() == 0) shopText = L"なし\n";
		mShopText->setText(shopText);

		//インジケーターの移動距離を設定
		mArrowMoveLength = mShopText->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowItemNum / mSaleItem.size();
		if (mSaleItem.size() < MaxShowItemNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mSaleItem.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//所持リソースのテキストを作成
	mResourceText = static_cast<TextComponent*>(mComponentLabels["resourceText"].pComponent);
	if(mResourceText)showResource();

	//アイテムの効果　テキスト	
	mItemEffectText = static_cast<TextComponent*>(mComponentLabels["itemEffectText"].pComponent);
	if(mItemEffectText) showItemEffect();

	//アイテム購入にかかるコスト
	mCostText = static_cast<TextComponent*>(mComponentLabels["costText"].pComponent);
	if(mCostText) showItemCost();
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
	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex--;													//選択アイテムのインデックスを減らす
		showItemEffect();													//選択アイテムの効果を表示
		showItemCost();														//選択アイテムのコストを表示
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");				//効果音を鳴らす

		//カーソルが上端の場合、矢印は動かさない
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));			//矢印を上に移動
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex++;													//選択アイテムのインデックスを増やす
		showItemEffect();													//選択アイテムの効果を表示
		showItemCost();														//選択アイテムのコストを表示
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");				//効果音を鳴らす

		//カーソルが下端の場合、矢印は動かさない
		if (mSelectedIndex > mScrollOffset + MaxShowItemNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));				//矢印を下に移動	
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
}

void ShopMenu::showItemEffect()
{
	//アイテムデータを取得
	if (mSaleItem.size() == 0) {
		std::wstring description = L" ";
		mItemEffectText->setText(description);
		return;
	}
	const auto& itemData = mItemManager.getItemData(mSaleItem[mSelectedIndex]);
	//アイテムの効果を表示
	std::wstring description = Utility::stringToWString(itemData.description);
	mItemEffectText->setText(description);
}

void ShopMenu::showItemCost()
{
	//アイテムデータを取得
	if (mSaleItem.size() == 0) {
		std::wstring costText = L" ";
		mCostText->setText(costText);
		return;
	}
	const auto& itemData = mItemManager.getItemData(mSaleItem[mSelectedIndex]);
	//アイテムのコストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < itemData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(itemData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" : " + std::to_wstring(itemData.price[i]);
	}
	costText += L"\n";
	mCostText->setText(costText);
}

void ShopMenu::showResource()
{
	//リソースデータの取得
	auto resourceData = mItemManager.getResourceData();
	//所持リソースの表示
	std::wstring resourceText = L"所持リソース\n";
	for (auto& data : resourceData) {
		if (data.second.num <= 0) continue;
		resourceText += Utility::stringToWString(data.second.name) + L" : " + std::to_wstring(mItemManager.getResourceNum(data.second.id)) + L"  ";
	}
	resourceText += L"\n";
	mResourceText->setText(resourceText);
}

void ShopMenu::buyItem(int index) {
	auto& playerData = mPlayerManager.getPlayerData();
	if (playerData.storageSize <= playerData.inventory.size()) {
		mScene.getGame().getAudioManager().playSE("UI_CANCEL");
		return;
	}

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

	mScene.getGame().getAudioManager().playSE("PURCHASE");
	//インベントリにアイテムを追加
	mPlayerManager.addInventory(itemData.id);

	//UI
	showResource();	//所持リソースの反映
	mScene.updateStatusWindow();	//所持アイテムの反映
}


