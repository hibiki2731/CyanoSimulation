#include "GameOverScene.h"
#include "Game.h"
#include "Graphic.h"
#include "input.h"
#include "SceneManager.h"
#include "TextComponent.h"
#include "AudioManager.h"
#include "SpriteComponent.h"

GameOverScene::GameOverScene(Game& game)
	:Scene(game)
{
}

void GameOverScene::onEnter()
{
	auto gameOverUI = std::make_unique<GameOverUI>(*this);
	addActor(std::move(gameOverUI));

	//BGM再生
	mGame.getAudioManager().playBGM("BGM_GAMEOVER");
}

void GameOverScene::onExit()
{
}

GameOverUI::GameOverUI(GameOverScene& scene)
	:Actor(scene)
{
	//背景
	auto background = std::make_unique<SpriteComponent>(*this);
	background->create("assets/picture/GameOverBackground.png");
	background->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(background));

	//テキスト
	auto text = std::make_unique<TextComponent>(*this, 90.0f);
	text->setFontSize(80.0f);
	text->setBaseLine(Graphic::ClientWidth * 0.33f, Graphic::ClientHeight * 0.4f);
	std::wstring message = L"GAME OVER\n";
	text->setText(message);
	text->setTextColor(D2D1::ColorF::Red);
	text->showText();
	addComponent(std::move(text));
}

void GameOverUI::inputActor()
{
	if (isKeyJustPressed(VK_RETURN)) {
		getScene().getGame().getSceneManager().transitToTitle();
	}
}
