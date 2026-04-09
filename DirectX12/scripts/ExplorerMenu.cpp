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
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
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


