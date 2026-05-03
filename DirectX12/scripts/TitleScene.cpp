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

	mDebugFlag = true;
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
	: Object(scene, "TitleUI")
{
	addComponentLabel("startText", "TextComponent");
	mTimer = 0;
	isStarting = false;
	mStartSEVoice = nullptr;

	applyComponentLabel();
}

void TitleUI::applyComponentLabel()
{
	mStartText = static_cast<TextComponent*>(mComponentLabels["startText"].pComponent);
}

void TitleUI::inputActor()
{
	if ((isKeyJustPressed(VK_RETURN) || isKeyJustPressed('K')) && !isStarting) {
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
