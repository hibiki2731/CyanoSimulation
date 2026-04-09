#include "MessageWindow.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Player.h"
#include "ItemManager.h"
#include "Scene.h"
#include "PlayerManager.h"

MessageWindow::MessageWindow(Scene& scene) : Actor(scene)
{
	
	std::wstring message = L"初期化";
	mMessage = message;
	auto text = std::make_unique<TextComponent>(*this, 0.9f);
	text->setText(mMessage);
	text->setPosition(100.0f, 10.0f);
	text->setFontSize(24.0f);
	text->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
	text->showText();
	mText = text.get();
	addComponent(std::move(text));

	auto window = std::make_unique<SpriteComponent>(*this, 1.0f);
	window->create("assets\\picture\\UI2\\PNG\\Default\\panel_brown.png");
	window->setBordarSize(24.0f);
	window->setSpriteSize(XMFLOAT2(600.0f, 100.0f));
	addComponent(std::move(window));

	mPlayer = nullptr;
	mPlayerManager = nullptr;
}

void MessageWindow::setPlayer(Player* player)
{
	mPlayer = player;
}

void MessageWindow::setPlayerManager(PlayerManager* playerManager) {
	mPlayerManager = playerManager;
}

void MessageWindow::updateActor()
{
	//デバッグ用
	std::wstring message;
	if (mPlayerManager) {
		message += L"HP: " + std::to_wstring(mScene.getGame().getPlayerManager().getPlayerData().hp) + L" ";
	}
	else if (mPlayer) {
		message += L"HP: " + std::to_wstring(mPlayer->getHP()) + L" STR: " + std::to_wstring(mPlayer->getPower())
			+ L" DEF: " + std::to_wstring(mPlayer->getDefense()) + L" ACTION_LIMIT" + std::to_wstring(mPlayer->getAP()) + L"\n";
	}

	message += L"G:" + std::to_wstring(mScene.getGame().getItemManager().getResourceNum("GOLD")) + L"\n";
	mMessage = message;
	mText->setText(mMessage);
	mText->showText();  //マルチスレッド化したい
}
