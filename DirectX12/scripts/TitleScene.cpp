#include "TitleScene.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "ItemManager.h"
#include "PlayerManager.h"
#include "input.h"
#include "SceneManager.h"
#include "AudioManager.h"
#include "GUIDebugger.h"

TitleScene::TitleScene(Game& game)
	: Scene(game),
	mItemManager(game.getItemManager()),
	mPlayerManager(game.getPlayerManager()),
	mAudioManager(game.getAudioManager())
{
}

void TitleScene::onEnter()
{

	mGame.getGraphic().startFadeIn(1.0f);

	auto titleUI = std::make_unique<TitleUI>(*this);
	addActor(std::move(titleUI));

	mAudioManager.playBGM("BGM_TITLE");
	
}

void TitleScene::onExit()
{
	mItemManager.loadItemData();
	mPlayerManager.loadPlayerData();
}

TitleUI::TitleUI(TitleScene& scene)
	: Actor(scene)
{
	//タイトルテキスト
	std::string structName = "TitleText";
	auto titleText = std::make_unique<TextComponent>(*this, 0.0f);
	titleText->loadFileAndCreate(structName);
#ifdef _DEBUG
	titleText->activateControll(structName);
#endif
	addComponent(std::move(titleText));

	//開始する方法
	structName = "StartText";
	auto startText = std::make_unique<TextComponent>(*this, 0.0f);
	startText->loadFileAndCreate(structName);
	mStartText = startText.get();
#ifdef _DEBUG
	startText->activateControll(structName);
#endif

	addComponent(std::move(startText));

	auto background = std::make_unique<SpriteComponent>(*this, 100.0f);
	background->create("assets/picture/TitleBackgroundLow.png");
	background->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(background));

	mTimer = 0;
	isStarting = false;
	mStartSEVoice = nullptr;
}

void TitleUI::inputActor()
{
	if (isKeyJustPressed(VK_RETURN) && !isStarting) {
		startTransit();
	}
}

void TitleUI::updateActor()
{
	mTimer++;
	if (isStarting) {
		if (mTimer % 5 == 0) {
			if ((mTimer / 5 ) % 2 == 0) {
				//透明
				mStartText->setTextColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
			}
			else {
				mStartText->setTextColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
			}
		}
	

		XAUDIO2_VOICE_STATE state;
		mStartSEVoice->GetState(&state);
		if (state.BuffersQueued == 0) {
			getScene().getGame().getSceneManager().transitToTown();
		}
	}
}

void TitleUI::startTransit()
{
	isStarting = true;
	mScene.getGame().getAudioManager().pauseBGM();
	mStartSEVoice = mScene.getGame().getAudioManager().playSE("TITLE_FOOTSTEP");
	mScene.getGame().getGraphic().startFadeOut(2.0f);
	mTimer = 0;
}
