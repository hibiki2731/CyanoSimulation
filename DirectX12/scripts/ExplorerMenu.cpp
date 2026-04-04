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

ExplorerMenu::ExplorerMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "ExplorerShopMenu", zDepth),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager())
{
	prepareCraftExplorer();
	mScrollOffset = 0;	

	static float fontSize =40.0f;
	static float lineSpace = 8.0f;

	//購入可能な武器と防具のテキストを作成
	std::wstring toolText = L"";
	auto textComponent = std::make_unique<TextComponent>(*this, zDepth - 1.0f);
	textComponent->setFontSize(fontSize);
	textComponent->setLineSpace(lineSpace);
	textComponent->setBaseLine(mPosition.x + 60.0f, mPosition.y + 75.0f);
	textComponent->setTextColor(D2D1::ColorF::Black);
	for (const auto& toolID : mTools) {
		const auto& toolData = mItemManager.getExplorerData(toolID);
		toolText += Utility::stringToWString(toolData.name) + L"\n";
	}
	if (toolText.size() == 0) toolText = L"なし\n";
	textComponent->setText(toolText);
	textComponent->showText();
	mToolText = textComponent.get();
	addComponent(std::move(textComponent));

	//矢印の移動距離を設定
	mArrowMoveLength = fontSize + lineSpace;

	//スクロールバー
	//下矢印
	auto downArrow = std::make_unique<SpriteComponent>(*this);
	downArrow->create("assets/picture/UI2/PNG/Default/minimap_arrow_a.png");
	downArrow->setPosition(XMFLOAT3(80.0f, 175.0f + MaxShowToolNum * 48.0f - 8.0f, zDepth - 0.5f));
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
	float arrowDistance = 48.0f * MaxShowToolNum - 38.0f;
	float height = arrowDistance * MaxShowToolNum / mPlayerManager.getPlayerData().weaponInventory.size();
	if (mPlayerManager.getPlayerData().weaponInventory.size() < MaxShowToolNum) height = arrowDistance;
	mScrollBarMoveLength = arrowDistance / mPlayerManager.getPlayerData().weaponInventory.size();
	scrollBar->setSpriteSize(XMFLOAT2(25.0f, height));
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


