#include "TownManager.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "Game.h"
#include "Graphic.h"
#include "SceneManager.h"
#include "json.hpp"
#include "MyUtility.h"
#include "input.h"
#include "MessageWindow.h"
#include "ItemManager.h"
#include "fstream"
#include "filesystem"
#include <iostream>
#include "PlayerManager.h"
#include "InnMenu.h"
#include "ShopMenu.h"
#include "ForgeMenu.h"
#include "ExplorerMenu.h"
#include "StatusMenu.h"

Menu::Menu(Game* game, std::string windowName, float zDepth) : Actor(game)
{
	mSelectedIndex = 0;
	mMaxIndex = 0;
	
	game->getTownManager()->pushMenu(this);
	initComponent(windowName, zDepth);
}

void Menu::inputMenu() {

	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex == 0) return;
		mSelectedIndex--;
		mArrow->movePositon(XMFLOAT2(0.0f, -mArrowMoveLength));
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex == mMaxIndex - 1) return;
		mSelectedIndex++;
		mArrow->movePositon(XMFLOAT2(0.0f, mArrowMoveLength));
	}		
}

void Menu::initComponent(std::string windowName, float zDepth)
{
	//jsonファイルからテキストウィンドウのパラメータを読み込む
	nlohmann::json textWindowData;
	std::ifstream file("assets\\data\\townMenuData.json");
	assert(!file.fail());
	file >> textWindowData;

	////ウィンドウの背景
	auto window = std::make_unique<SpriteComponent>(this, zDepth);
	window->create(textWindowData[windowName]["spriteFileName"].get<std::string>());
	window->setBordarSize(textWindowData[windowName].value("borderSize", 24.0f));
	window->setSpriteSize(XMFLOAT2(textWindowData[windowName]["width"].get<float>(), textWindowData[windowName]["height"].get<float>()));
	XMFLOAT3 pos = XMFLOAT3(textWindowData[windowName]["x"].get<float>(), textWindowData[windowName]["y"].get<float>(), zDepth);
	window->setPosition(pos);
	addComponent(std::move(window));

	//見出し
	std::wstring titleName = Utility::stringToWString(textWindowData[windowName].value("title", ""));
	if (titleName != L"") {
		float fontSize = textWindowData[windowName]["titleFontSize"].get<float>();
		auto title = std::make_unique<TextComponent>(this, zDepth - 0.5f);
		title->setText(titleName);
		title->setBaseLine(pos.x + textWindowData[windowName]["titleOffsetX"].get<float>(), pos.y + textWindowData[windowName]["titleOffsetY"].get<float>());
		title->setFontSize(fontSize);
		title->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		title->showText();
		addComponent(std::move(title));
	}

	//テキスト
	std::wstring message = Utility::stringToWString(textWindowData[windowName].value("text", ""));
	if (message != L"") {
		float fontSize = textWindowData[windowName]["fontSize"].get<float>();
		float lineSpace = textWindowData[windowName].value("lineSpace", 0.0f);
		auto text = std::make_unique<TextComponent>(this, zDepth - 0.5f);
		text->setText(message);
		text->setBaseLine(pos.x + textWindowData[windowName]["textOffsetX"].get<float>(), pos.y + textWindowData[windowName]["textOffsetY"].get<float>());
		text->setFontSize(fontSize);
		text->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		text->setLineSpace(lineSpace);
		text->showText();
		addComponent(std::move(text));
		mArrowMoveLength = fontSize + lineSpace;
	}

	//矢印
	auto arrow = std::make_unique<SpriteComponent>(this);
	arrow->create(textWindowData[windowName]["arrowFileName"].get<std::string>());
	arrow->setBordarSize(0.0f);
	arrow->setSpriteSize(XMFLOAT2(textWindowData[windowName]["arrowWidth"].get<float>(), textWindowData[windowName]["arrowHeight"].get<float>()));
	arrow->setPosition(XMFLOAT3(pos.x + textWindowData[windowName]["arrowOffsetX"].get<float>(), pos.y + textWindowData[windowName]["arrowOffsetY"].get<float>(), zDepth - 1.0f));
	arrow->setRotation(-XM_PIDIV2);
	mArrow = arrow.get();
	addComponent(std::move(arrow));


}

//各種メニューのコンストラクタ
MainMenu::MainMenu(Game* game, float zDepth) : Menu(game, "MainMenu", zDepth)
{
	mMaxIndex = 5;
}

//各種メニューのupdate
void MainMenu::selectedAct() {
	switch (mSelectedIndex) {
	case 0: {
		auto inn = std::make_unique<InnMenu>(mGame, 97.0f);
		mGame->addActor(std::move(inn));
		break;
	}
	case 1: {
		auto shop = std::make_unique<ShopMenu>(mGame, 97.0f);
		mGame->addActor(std::move(shop));
		break;
	}
	case 2: {
		auto forge = std::make_unique<ForgeMenu>(mGame, 97.0f);
		mGame->addActor(std::move(forge));
		break;
	}
	case 3: {
		auto explorer = std::make_unique<ExplorerMenu>(mGame, 97.0f);
		mGame->addActor(std::move(explorer));
		break;
	}
	case 4: {
		mGame->getSceneManager()->transitToMap();
	}
	}
}

//BackGround
BackGround::BackGround(Game* game) : Actor(game)
{
	auto window = std::make_unique<SpriteComponent>(this);
	window->create("assets\\picture\\town.png");
	window->setBordarSize(0.0f);
	window->setSpriteSize(XMFLOAT2(game->getGraphic()->getClientWidth(), game->getGraphic()->getClientHeight()));
	addComponent(std::move(window));
}

//TownManager
TownManager::TownManager(Game* game)
{
	mGame = game;
	isTown = false;
	isSelected = false;
	mBg = nullptr;
}

void TownManager::input()
{
	if (!isTown) return;

	if (!mMenuStack.empty()) mMenuStack.top()->inputMenu();

	if (isKeyJustPressed(VK_RETURN)) isSelected = true;

	if (isKeyJustPressed(VK_ESCAPE) && mMenuStack.size() > 1) {
		popMenu();
	}

	if (isKeyJustPressed('E') && !isStatusMenu) {
		isStatusMenu = true;
		auto status = std::make_unique<StatusMenu>(mGame, 50.0f);
		mGame->addActor(std::move(status));
	}
}

void TownManager::update()
{
	//シーンがTOWNに切り替わった際の処理
	if (!isTown && mGame->getSceneManager()->getCurrentScene() == SceneType::TOWN) {
		isTown = true;

		auto bg = std::make_unique<BackGround>(mGame);
		mBg = bg.get();
		mGame->addActor(std::move(bg));

		auto textWindow = std::make_unique<MainMenu>(mGame, 99.0f);
		mGame->addActor(std::move(textWindow));

		auto playerWindow = std::make_unique<MessageWindow>(mGame);
		mGame->addActor(std::move(playerWindow));
	}

	//シーンがTOWNの際の処理
	if (isTown) {
		//決定キーが押された場合の処理
		if (!mMenuStack.empty() && isSelected) {
			isSelected = false;
			mMenuStack.top()->selectedAct();
		}
		mMenuStack.top()->updateMenu();

	}

	//シーンがTONWN以外に切り替わった際の処理
	if (isTown && mGame->getSceneManager()->getCurrentScene() != SceneType::TOWN) {
		isTown = false;
		//スタックを空にする
		for (int i = 0; i < mMenuStack.size(); i++) popMenu();
		mBg->setState(Actor::State::Dead);

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

