#include "MessageWindow.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "SceneManager.h"
#include "Graphic.h"
#include "Game.h"
#include "Player.h"
#include "MapManager.h"
#include "ItemManager.h"
#include "PlayerManager.h"

MessageWindow::MessageWindow(Game* game) : Actor(game)
{
	
	std::wstring message = L"初期化";
	mMessage = message;
	auto text = std::make_unique<TextComponent>(this, 0.9f);
	text->setText(mMessage);
	text->setBaseLine(100.0f, 10.0f);
	text->setFontSize(24.0f);
	text->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	text->showText();
	mText = text.get();
	addComponent(std::move(text));

	auto window = std::make_unique<SpriteComponent>(this, 1.0f);
	window->create("assets\\picture\\UI2\\PNG\\Default\\panel_brown.png");
	window->setBordarSize(24.0f);
	window->setSpriteSize(XMFLOAT2(600.0f, 100.0f));
	addComponent(std::move(window));
}

void MessageWindow::inputActor()
{
}

void MessageWindow::updateActor()
{
	//デバッグ用
	std::wstring message;
	if (mGame->getSceneManager()->getCurrentScene() == SceneType::TOWN) {
		message += L"HP: " + std::to_wstring(mGame->getPlayerManager()->getPlayerData().hp) + L" ";
	}
	else if (mGame->getSceneManager()->getCurrentScene() == SceneType::MAP) {
		message += L"HP: " + std::to_wstring(mGame->getMapManager()->getPlayer()->getHP()) + L" STR: " + std::to_wstring(mGame->getMapManager()->getPlayer()->getPower())
			+ L" DEF: " + std::to_wstring(mGame->getMapManager()->getPlayer()->getDefense()) + L" ACTION_LIMIT" + std::to_wstring(mGame->getMapManager()->getPlayer()->getActionLimit()) + L"\n";
	}

	message += L"G:" + std::to_wstring(mGame->getItemManager()->getResourceNum("GRASS")) + L"\n";
	mMessage = message;
	mText->setText(mMessage);
	mText->showText();  //マルチスレッド化したい
}
