#include "Menu.h"
#include "TownScene.h"
#include "input.h"
#include "Game.h"
#include "AudioManager.h"
#include "SpriteComponent.h"
#include "json.hpp"
#include "MyUtility.h"
#include "TextComponent.h"
#include <fstream>

Menu::Menu(TownScene& scene, std::string windowName, float zDepth) 
	: Actor(scene),
	mScene(scene)
{
	mSelectedIndex = 0;
	mMaxIndex = 0;
	
	scene.pushMenu(this);
	initComponent(windowName, zDepth);
}

void Menu::inputMenu() {

	if (isKeyJustPressed(VK_UP)) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
	}

	if (isKeyJustPressed(VK_DOWN)) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
		mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
		mScene.getGame().getAudioManager().playSE("UI_MOVE1");
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
	auto window = std::make_unique<SpriteComponent>(*this, zDepth);
	window->create(textWindowData[windowName]["spriteFileName"].get<std::string>());
	window->setBordarSize(textWindowData[windowName].value("borderSize", 24.0f));
	window->setSpriteSize(XMFLOAT2(textWindowData[windowName]["width"].get<float>(), textWindowData[windowName]["height"].get<float>()));
	mPosition = XMFLOAT3(textWindowData[windowName]["x"].get<float>(), textWindowData[windowName]["y"].get<float>(), zDepth);
	window->setPosition(mPosition);
	addComponent(std::move(window));

	//見出し
	std::wstring titleName = Utility::stringToWString(textWindowData[windowName].value("title", ""));
	if (titleName != L"") {
		float fontSize = textWindowData[windowName]["titleFontSize"].get<float>();
		auto title = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
		title->setText(titleName);
		title->setBaseLine(mPosition.x + textWindowData[windowName]["titleOffsetX"].get<float>(), mPosition.y + textWindowData[windowName]["titleOffsetY"].get<float>());
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
		auto text = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
		text->setText(message);
		text->setBaseLine(mPosition.x + textWindowData[windowName]["textOffsetX"].get<float>(), mPosition.y + textWindowData[windowName]["textOffsetY"].get<float>());
		text->setFontSize(fontSize);
		text->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		text->setLineSpace(lineSpace);
		text->showText();
		addComponent(std::move(text));
		mArrowMoveLength = fontSize + lineSpace;
	}

	//矢印
	auto arrow = std::make_unique<SpriteComponent>(*this);
	arrow->create(textWindowData[windowName]["arrowFileName"].get<std::string>());
	arrow->setBordarSize(0.0f);
	arrow->setSpriteSize(XMFLOAT2(textWindowData[windowName]["arrowWidth"].get<float>(), textWindowData[windowName]["arrowHeight"].get<float>()));
	arrow->setPosition(XMFLOAT3(mPosition.x + textWindowData[windowName]["arrowOffsetX"].get<float>(), mPosition.y + textWindowData[windowName]["arrowOffsetY"].get<float>(), zDepth - 1.0f));
	arrow->setRotation(-XM_PIDIV2);
	mArrow = arrow.get();
	addComponent(std::move(arrow));


}

