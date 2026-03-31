#include "TownManager.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Graphic.h"
#include "SceneManager.h"
#include "MyUtility.h"
#include "input.h"
#include "MessageWindow.h"
#include "PlayerManager.h"
#include "StatusMenu.h"
#include "AudioManager.h"
#include "MainMenu.h"

//BackGround
BackGround::BackGround(Game& game) : Actor(game)
{
	auto window = std::make_unique<SpriteComponent>(*this);
	window->create("assets\\picture\\town.png");
	window->setBordarSize(0.0f);
	window->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(window));
}

//TownManager
TownManager::TownManager(Game& game)
	: Scene(game),
	mGame(game)
{
	isTown = false;
	isSelected = false;
}

void TownManager::onEnter() {
	isTown = true;

	auto bg = std::make_unique<BackGround>(mGame);
	mActors.emplace_back(bg.get());
	mGame.addActor(std::move(bg));

	auto textWindow = std::make_unique<MainMenu>(mGame, *this, 99.0f);
	mGame.addActor(std::move(textWindow));

	auto playerWindow = std::make_unique<MessageWindow>(mGame);
	playerWindow->setPlayerManager(mGame.getPlayerManager());
	mActors.emplace_back(playerWindow.get());
	mGame.addActor(std::move(playerWindow));

	mGame.getAudioManager()->playBGM("BGM_TOWN");
}

void TownManager::onExit() {
	isTown = false;
	//スタックを空にする
	for (int i = 0; i < mMenuStack.size(); i++) popMenu();
	for (auto& actor : mActors) {
		actor->setState(Actor::State::Dead);
	}
}

void TownManager::input()
{
	if (!isTown) return;

	if (!mMenuStack.empty()) mMenuStack.top()->inputMenu();

	if (isKeyJustPressed(VK_RETURN)) {
		isSelected = true;
	}

	if (isKeyJustPressed(VK_ESCAPE) && mMenuStack.size() > 1) {
		popMenu();
		mGame.getAudioManager()->playSE("UI_WINDOW_CLOSE");
	}

	if (isKeyJustPressed('E') && !isStatusMenu) {
		isStatusMenu = true;
		auto status = std::make_unique<StatusMenu>(mGame, *this, 50.0f);
		mGame.addActor(std::move(status));
		mGame.getAudioManager()->playSE("UI_WINDOW_OPEN");
	}
}

void TownManager::update()
{
	//シーンがTOWNの際の処理
	if (isTown) {
		//決定キーが押された場合の処理
		if (!mMenuStack.empty() && isSelected) {
			isSelected = false;
			mMenuStack.top()->selectedAct();
		}
		mMenuStack.top()->updateMenu();

	}
}

void TownManager::pushMenu(Menu* menu)
{
	mMenuStack.push(menu);
}

void TownManager::popMenu()
{
	if (mMenuStack.empty()) return;

	mMenuStack.top()->setState(Actor::State::Dead);
	mMenuStack.pop();

}

void TownManager::exitStatusMenu()
{
	isStatusMenu = false;
}

