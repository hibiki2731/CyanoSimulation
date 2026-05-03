#include "MainMenu.h"
#include "Game.h"
#include "TownScene.h"
#include "AudioManager.h"
#include "InnMenu.h"
#include "ShopMenu.h"
#include "ForgeMenu.h"
#include "ExplorerMenu.h"
#include "SceneManager.h"
#include "Graphic.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "input.h"

//各種メニューのコンストラクタ
MainMenu::MainMenu(TownScene& scene, float zDepth) 
	: Menu(scene, "MainMenu", zDepth)
{
	mMaxIndex = 5;
	movingToDungeon = false;

	addComponentLabel("descriptor", "TextComponent");
	applyComponentLabel();
}

void MainMenu::applyComponentLabel()
{
	mDescriptor = static_cast<TextComponent*>(mComponentLabels["descriptor"].pComponent);
	if (mDescriptor) {
		mDescriptorDefaultZ = mDescriptor->getPosZ();
		updateDescriptor();
	}
}

void MainMenu::updateMenu()
{
	if (movingToDungeon && mScene.getGame().getGraphic().isFinishedFade()) {
		mScene.getGame().getSceneManager().transitToMap();
	}
}

void MainMenu::updateActor()
{
	if (mDescriptor) {
		//メニューがアクティブでないとき
		if (mScene.getCurrentMenu() != this) {
			mDescriptor->setPosZ(200.0f);
		}
		else
			mDescriptor->setPosZ(mDescriptorDefaultZ);
	}
}

void MainMenu::inputMenu()
{
	if (mArrow) {
		if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
			if (mSelectedIndex <= 0) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex--;
			mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}

		if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
			if (mSelectedIndex >= mMaxIndex - 1) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex++;
			mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}
	}
}

//各種メニューのupdate
void MainMenu::selectedAct() {
	mScene.getGame().getAudioManager().playSE("UI_WINDOW_OPEN");
	switch (mSelectedIndex) {
	case 0: {
		auto inn = std::make_unique<InnMenu>(mScene, 97.0f);
		mScene.addActor(std::move(inn));
		break;
	}
	case 1: {
		auto shop = std::make_unique<ShopMenu>(mScene, 97.0f);
		mScene.addActor(std::move(shop));
		break;
	}
	case 2: {
		auto forge = std::make_unique<ForgeMenu>(mScene, 97.0f);
		mScene.addActor(std::move(forge));
		break;
	}
	case 3: {
		auto explorer = std::make_unique<ExplorerMenu>(mScene, 97.0f);
		mScene.addActor(std::move(explorer));
		break;
	}
	case 4: {
		startTransit();
		break;
	}
	}
}

void MainMenu::startTransit()
{
	movingToDungeon = true;
	mScene.getGame().getAudioManager().finishAllSounds();
	mScene.getGame().getAudioManager().playSE("MOVE_TO_DUNGEON");
	mScene.getGame().getGraphic().startFadeOut(1.0f);
}

void MainMenu::updateDescriptor()
{
	if (!mDescriptor) return;

	std::wstring text;
	switch (mSelectedIndex) {
	case 0:
		text = L"HPの回復やセーブが出来ます。\n";
		break;
	case 1:
		text = L"アイテムの購入が出来ます。\n";
		break;
	case 2:
		text = L"装備品の作製が出来ます。\n";
		break;
	case 3:
		text = L"探索道具を購入できます。\n";
		break;
	case 4:
		text = L"ダンジョンへ挑戦します。\n";
		break;
	}

	mDescriptor->setText(text);
}

