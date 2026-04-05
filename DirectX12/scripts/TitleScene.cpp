#include "TitleScene.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "ItemManager.h"
#include "PlayerManager.h"
#include "input.h"
#include "SceneManager.h"

TitleScene::TitleScene(Game& game)
	: Scene(game),
	mItemManager(game.getItemManager()),
	mPlayerManager(game.getPlayerManager())
{
}

void TitleScene::onEnter()
{
	auto titleUI = std::make_unique<TitleUI>(*this);
	addActor(std::move(titleUI));
}

void TitleScene::onExit()
{
	mItemManager.loadItemData();
	mPlayerManager.loadPlayerData();
}

void TitleScene::inputScene()
{
	if (isKeyJustPressed(VK_RETURN)) {
		getGame().getSceneManager().transitToTown();
	}
}

TitleUI::TitleUI(TitleScene& scene)
	: Actor(scene)
{
	auto titleText = std::make_unique<TextComponent>(*this, 0.0f);
	titleText->setFontSize(80.0f);
	titleText->setBaseLine(Graphic::ClientWidth * 0.35f, Graphic::ClientHeight * 0.3f);
	std::wstring titleStr = L"TITLE\n";
	titleText->setText(titleStr);
	titleText->showText();
	addComponent(std::move(titleText));

	auto background = std::make_unique<SpriteComponent>(*this, 100.0f);
	background->create("assets/picture/TitleBackground.png");
	background->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(background));
}
