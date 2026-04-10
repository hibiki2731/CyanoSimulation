#include "TownScene.h"
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
#include "StatusWindow.h"

//BackGround
BackGround::BackGround(Scene& scene) : Actor(scene)
{
	auto window = std::make_unique<SpriteComponent>(*this);
	window->create("assets\\picture\\town.png");
	window->setBordarSize(0.0f);
	window->setSpriteSize(XMFLOAT2(Graphic::ClientWidth, Graphic::ClientHeight));
	addComponent(std::move(window));
}

//TownScene
TownScene::TownScene(Game& game)
	: Scene(game)
{
	isSelected = false;
}

void TownScene::onEnter() {

	//背景
	auto bg = std::make_unique<BackGround>(*this);
	addActor(std::move(bg));

	//メインメニュー
	auto mainMenu = std::make_unique<MainMenu>(*this, 99.0f);
	addActor(std::move(mainMenu));

	//ステータスウィンドウ
	auto statusWindow = std::make_unique<StatusWindow>(*this, 99.0f);
	mStatusWindow = statusWindow.get();
	addActor(std::move(statusWindow));

	//フェードイン
	mGame.getGraphic().startFadeIn(1.0f);

	mGame.getAudioManager().playBGM("BGM_TOWN");
}

void TownScene::onExit() {
	//スタックを空にする
	for (int i = 0; i < mMenuStack.size(); i++) popMenu();
	refreshActors();
	mStatusWindow = nullptr;
}

void TownScene::inputScene()
{
	if (!mMenuStack.empty()) mMenuStack.top()->inputMenu();

	if (isKeyJustPressed(VK_RETURN)) {
		isSelected = true;
	}

	if (isKeyJustPressed(VK_ESCAPE) && mMenuStack.size() > 1) {
		popMenu();
		mGame.getAudioManager().playSE("UI_WINDOW_CLOSE");
	}

	if (isKeyJustPressed('E') && !isStatusMenu) {
		isStatusMenu = true;
		auto status = std::make_unique<StatusMenu>(*this, 50.0f);
		addActor(std::move(status));
		mGame.getAudioManager().playSE("UI_WINDOW_OPEN");
	}
}

void TownScene::updateScene()
{
	//決定キーが押された場合の処理
	if (!mMenuStack.empty() && isSelected) {
		isSelected = false;
		mMenuStack.top()->selectedAct();
	}
	mMenuStack.top()->updateMenu();
}

void TownScene::pushMenu(Menu* menu)
{
	mMenuStack.push(menu);
}

void TownScene::popMenu()
{
	if (mMenuStack.empty()) return;

	mMenuStack.top()->setState(Actor::State::Dead);
	mMenuStack.pop();

}

void TownScene::exitStatusMenu()
{
	isStatusMenu = false;
}

void TownScene::updateStatusWindow()
{
	mStatusWindow->updateStatus();
}

