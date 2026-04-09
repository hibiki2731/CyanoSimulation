#include "TownScene.h"
#include "ExplorerMenu.h"
#include "AudioManager.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "MyUtility.h"
#include "input.h"
#include <fstream>
#include "json.hpp"

ExplorerMenu::ExplorerMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ExplorerShopMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftExplorer();
	mScrollOffset = 0;	

	//ファイル読み込み
	std::ifstream spriteFile("assets\\data\\spriteData.json");
	nlohmann::json spriteJson;
	spriteFile >> spriteJson;
	std::ifstream textFile("assets\\data\\textData.json");
	nlohmann::json textJson;
	textFile >> textJson;

	//購入可能な武器と防具のテキストを作成
	std::string structName = "ExplorerMenuScrollText";
	std::wstring toolText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->loadFileAndCreate(structName);
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& toolID : mTools) {
		const auto& toolData = mItemManager.getExplorerData(toolID);
		toolText += Utility::stringToWString(toolData.name) + L"\n";
	}
	if (toolText.size() == 0) toolText = L"なし\n";
	textComponent->setText(toolText);
	textComponent->showText();
#ifdef _DEBUG
	textComponent->activateControll(structName);
#endif
	mToolText = textComponent.get();
	addComponent(std::move(textComponent));

	//矢印の移動距離を設定
	mArrowMoveLength = textJson[structName]["lineSpace"];

	//スクロールバー
	//下矢印
	structName = "ExplorerMenuDownArrow";
	auto downArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	downArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	downArrow->activateControll(structName);
#endif
	addComponent(std::move(downArrow));

	//上矢印
	structName = "ExplorerMenuUpArrow";
	auto upArrow = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	upArrow->loadFileAndCreate(structName);
#ifdef _DEBUG
	upArrow->activateControll(structName);
#endif
	addComponent(std::move(upArrow));

	//スクロールバー
	structName = "ExplorerMenuScrollBar";
	auto scrollBar = std::make_unique<SpriteComponent>(*this, zDepth - 1.0f);
	scrollBar->loadFileAndCreate(structName);
	float maxHeight = scrollBar->getSpriteSize().y;
	float height = maxHeight * MaxShowToolNum / mTools.size();
	if (mTools.size() < MaxShowToolNum) height = maxHeight;
	mScrollBarMoveLength = maxHeight / mTools.size();
	scrollBar->setSpriteSize(XMFLOAT2(scrollBar->getSpriteSize().x, height));
#ifdef _DEBUG
	scrollBar->activateControll(structName);
#endif
	mScrollBar = scrollBar.get();
	addComponent(std::move(scrollBar));

	//所持リソース表示用のキャンバス
	structName = "ResourceCanvas";
	auto resourceCanvas = std::make_unique<SpriteComponent>(*this, zDepth);
	resourceCanvas->loadFileAndCreate(structName);
#ifdef _DEBUG
	resourceCanvas->activateControll(structName);
#endif
	addComponent(std::move(resourceCanvas));

	//所持リソースのテキストを作成
	structName = "ResourceText";
	auto resourceText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	resourceText->loadFileAndCreate(structName);
	resourceText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	resourceText->activateControll(structName);
#endif
	mResourceText = resourceText.get();
	showResource();
	addComponent(std::move(resourceText));

	//装備の効果
	structName = "EffectCanvas";
	auto toolEffectCanvas = std::make_unique<SpriteComponent>(*this, zDepth );
	toolEffectCanvas->loadFileAndCreate(structName);

#ifdef _DEBUG
	toolEffectCanvas->activateControll(structName);
#endif
	addComponent(std::move(toolEffectCanvas));

	//装備の効果　テキスト	
	structName = "EffectText";
	auto toolEffectText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	toolEffectText->loadFileAndCreate(structName);
	toolEffectText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	toolEffectText->activateControll(structName);
#endif
	mToolEffectText = toolEffectText.get();
	showToolEffect();
	addComponent(std::move(toolEffectText));

	//装備作製にかかるコスト
	structName = "CostText";
	auto costText = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
	costText->loadFileAndCreate(structName);
	costText->setTextColor(D2D1::ColorF::Black);
#ifdef _DEBUG
	costText->activateControll(structName);
#endif
	mCostText = costText.get();
	showCraftCost();
	addComponent(std::move(costText));
}

void ExplorerMenu::selectedAct()
{
	craftExplorer(mSelectedIndex);
}

void ExplorerMenu::updateMenu(){
	//カーソルが下端に来たら、テキストを下にスライド
	if (mSelectedIndex > mScrollOffset + MaxShowToolNum - 1) {		
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

void ExplorerMenu::inputMenu()
{
	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		showCraftCost();
		showToolEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
		showCraftCost();
		showToolEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex > mScrollOffset + MaxShowToolNum  - 1) return;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void ExplorerMenu::prepareCraftExplorer()
{
	for (auto& data : mItemManager.getExplorerData()) {
		if (data.second.inPossession) continue;
		mTools.emplace_back(data.second.id);
	}
	mMaxIndex = mTools.size();
}

void ExplorerMenu::craftExplorer(int index)
{
	if (mTools.size() == 0) return;

	//リソースを消費
	const auto& explorerData = mItemManager.getExplorerData(mTools[index]);
	for (int i = 0; i < explorerData.costResourceID.size(); i++) {
		int possessedResource = mItemManager.getResourceNum(explorerData.costResourceID[i]);
		//消費リソース分持っていなかったら買えない
		if (explorerData.price[i] > possessedResource) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}

		//所持リソースを消費リソース分減らす
		mItemManager.subResource(explorerData.costResourceID[i], explorerData.price[i]);
	}

	mScene.getGame().getAudioManager().playSE("UI_ENTER");
	//陳列からアイテムを削除
	mTools.erase(mTools.begin() + index);
	mItemManager.setToolPossession(explorerData.id, true);
	//インベントリにアイテムを追加
	mPlayerManager.addExplorer(explorerData.id);
	refreshText();
	//プレイヤーデータを更新
	mPlayerManager.applyToolParamater();
	//UIに反映
	mScene.updateStatusWindow();
	showCraftCost();
	showToolEffect();

}

void ExplorerMenu::refreshText()
{
	//テキストの更新
	std::wstring toolText = L"";
	int showToolNum = min(mTools.size(), MaxShowToolNum);
	for (int i = mScrollOffset; i < mScrollOffset + showToolNum; i++) {
		const auto& toolData = mItemManager.getExplorerData(mTools[i]);
		toolText += Utility::stringToWString(toolData.name) + L"\n";
	}
	if (toolText.size() == 0) toolText = L"なし\n";
	mToolText->setText(toolText);
	mToolText->showText();

	//インデックスの更新
	mMaxIndex--;
	if (mSelectedIndex >= mMaxIndex && mSelectedIndex > 0) {
		mSelectedIndex--;
		if (mSelectedIndex < mScrollOffset) {
			mScrollOffset--;
			mScrollBar->movePosition(XMFLOAT2(0.0f, -mScrollBarMoveLength));
		}
		else mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}
}

void ExplorerMenu::showCraftCost()
{
	//道具のデータを取得
	if (mTools.size() == 0) {
		mCostText->setText(L" ");
		mCostText->showText();
		return;
	}
	const auto& toolData = mItemManager.getExplorerData(mTools[mSelectedIndex]);
	//道具の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < toolData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(toolData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" : " + std::to_wstring(toolData.price[i]);
	}
	costText += L"\n";
	mCostText->setText(costText);
	mCostText->showText();
}

void ExplorerMenu::showResource()
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
	mResourceText->showText();
}

void ExplorerMenu::showToolEffect()
{
	//道具データを取得
	if (mTools.size() == 0) {
		mToolEffectText->setText(L" ");
		mToolEffectText->showText();
		return;
	}
	const auto& toolData = mItemManager.getExplorerData(mTools[mSelectedIndex]);
	//道具の効果を表示
	std::wstring description = Utility::stringToWString(toolData.description);
	mToolEffectText->setText(description);
	mToolEffectText->showText();
}


