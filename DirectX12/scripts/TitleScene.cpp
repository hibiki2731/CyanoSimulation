#include "TitleScene.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "ItemManager.h"
#include "PlayerManager.h"
#include "input.h"
#include "SceneManager.h"
#include "AudioManager.h"

TitleScene::TitleScene(Game& game)
	: Scene(game),
	mItemManager(game.getItemManager()),
	mPlayerManager(game.getPlayerManager()),
	mAudioManager(game.getAudioManager())
{
}

void TitleScene::onEnter()
{
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
	auto titleText = std::make_unique<TextComponent>(*this, 0.0f);
	titleText->setFontSize(80.0f);
	titleText->setPosition(Graphic::ClientWidth * 0.4f, Graphic::ClientHeight * 0.3f);
	std::wstring titleStr = L"TITLE\n";
	titleText->setText(titleStr);
	titleText->showText();
	addComponent(std::move(titleText));

	//開始する方法
	auto startText = std::make_unique<TextComponent>(*this, 0.0f);
	startText->setFontSize(40.0f);
	startText->setPosition(Graphic::ClientWidth * 0.35f, Graphic::ClientHeight * 0.7f);
	std::wstring startStr = L"PRESS ENTER TO START\n";
	startText->setText(startStr);
	startText->showText();
	mStartText = startText.get();
	addComponent(std::move(startText));

	auto background = std::make_unique<SpriteComponent>(*this, 100.0f);
	background->create("assets/picture/TitleBackgroundLow.png");
	background->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(background));

	mTimer = 0;
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
	if (isStarting) {
		mTimer++;
		if (mTimer % 5 == 0) {
			if ((mTimer / 5 ) % 2 == 0) {
				//透明
				mStartText->setTextColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
				mStartText->showText();
			}
			else {
				mStartText->setTextColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
				mStartText->showText();
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
}
