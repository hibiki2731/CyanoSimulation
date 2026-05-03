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

	addComponentLabel("toolText", "TextComponent");
	addComponentLabel("scrollBar", "SpriteComponent");
	addComponentLabel("resourceText", "TextComponent");
	addComponentLabel("toolEffectText", "TextComponent");
	addComponentLabel("costText", "TextComponent");

	applyComponentLabel();
}

void ExplorerMenu::applyComponentLabel()
{
	//製作可能な道具を表示
	mToolText = static_cast<TextComponent*>(mComponentLabels["toolText"].pComponent);
	if (mToolText) {
		std::wstring toolText;
		for (const auto& toolID : mTools) {
			const auto& toolData = mItemManager.getExplorerData(toolID);
			toolText += Utility::stringToWString(toolData.name) + L"\n";
		}
		if (toolText.size() == 0) toolText = L"なし\n";
		mToolText->setText(toolText);

		//矢印の移動距離を設定
		mArrowMoveLength = mToolText->getLineSpace();
	}

	//スクロールバー
	mScrollBar = static_cast<SpriteComponent*>(mComponentLabels["scrollBar"].pComponent);
	if (mScrollBar) {
		float maxHeight = mScrollBar->getSpriteSize().y;
		float height = maxHeight * MaxShowToolNum / mTools.size();
		if (mTools.size() < MaxShowToolNum) height = maxHeight;
		mScrollBarMoveLength = maxHeight / mTools.size();
		mScrollBar->setSpriteSize(XMFLOAT2(mScrollBar->getSpriteSize().x, height));
	}

	//所持リソースのテキストを作成
	mResourceText = static_cast<TextComponent*>(mComponentLabels["resourceText"].pComponent);
	if(mResourceText) showResource();

	//道具の効果
	mToolEffectText = static_cast<TextComponent*>(mComponentLabels["toolEffectText"].pComponent);
	if(mToolEffectText) showToolEffect();

	//装備作製にかかるコスト
	mCostText = static_cast<TextComponent*>(mComponentLabels["costText"].pComponent);
	if(mCostText) showCraftCost();

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
	if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
		if (mSelectedIndex <= 0) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
		mSelectedIndex--;
		showCraftCost();
		showToolEffect();
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		if (mSelectedIndex < mScrollOffset) return;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
		if (mSelectedIndex >= mMaxIndex - 1) {
			mScene.getGame().getAudioManager().playSE("UI_CANCEL");
			return;
		}
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
	if (mTools.size() == 0) {
		mScene.getGame().getAudioManager().playSE("UI_CANCEL");
		return;
	}

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

	mScene.getGame().getAudioManager().playSE("FORGE");
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
	showResource();

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
		return;
	}
	const auto& toolData = mItemManager.getExplorerData(mTools[mSelectedIndex]);
	//道具の製作コストを表示
	std::wstring costText = L"消費リソース\n";
	for (int i = 0; i < toolData.costResourceID.size(); i++) {
		const auto& resourceData = mItemManager.getResourceData(toolData.costResourceID[i]);
		costText += Utility::stringToWString(resourceData.name) + L" " + std::to_wstring(toolData.price[i]) + L"  ";
	}
	costText += L"\n";
	mCostText->setText(costText);
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
}

void ExplorerMenu::showToolEffect()
{
	//道具データを取得
	if (mTools.size() == 0) {
		mToolEffectText->setText(L" ");
		return;
	}
	const auto& toolData = mItemManager.getExplorerData(mTools[mSelectedIndex]);
	//道具の効果を表示
	std::wstring description = Utility::stringToWString(toolData.description);
	mToolEffectText->setText(description);
}


