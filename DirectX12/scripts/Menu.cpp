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
	//スプライトデータの取得
	nlohmann::json spriteJson;
	std::ifstream spritefile("assets\\data\\spriteData.json");
	spritefile >> spriteJson;

	//テキストデータの取得
	nlohmann::json textJson;
	std::ifstream textfile("assets\\data\\textData.json");
	textfile >> textJson;

	////ウィンドウの背景
	auto window = std::make_unique<SpriteComponent>(*this, zDepth);
	window->create(spriteJson[windowName]["filePath"].get<std::string>());
	window->setBordarSize(spriteJson[windowName].value("borderSize", 24.0f));
	window->setSpriteSize(XMFLOAT2(spriteJson[windowName]["width"].get<float>(), spriteJson[windowName]["height"].get<float>()));
	mPosition = XMFLOAT3(spriteJson[windowName]["x"].get<float>(), spriteJson[windowName]["y"].get<float>(), zDepth);
	window->setPosition(mPosition);
#ifdef _DEBUG
	window->activateControll("assets\\data\\townMenuData.json", windowName);
#endif
	addComponent(std::move(window));

	//見出し
	std::wstring titleName = Utility::stringToWString(textJson[windowName + "Title"].value("title", ""));
	if (titleName != L"") {
		auto title = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
		title->setText(titleName);
		title->setBaseLine(textJson[windowName + "Title"]["x"].get<float>(), textJson[windowName + "Title"]["y"].get<float>());
		title->setFontSize(textJson[windowName + "Title"]["fontSize"].get<float>());
		title->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		title->showText();
		addComponent(std::move(title));
	}

	//テキスト
	std::wstring message = Utility::stringToWString(textJson[windowName + "Text"].value("text", ""));
	if (message != L"") {
		float fontSize = textJson[windowName + "Text"]["fontSize"].get<float>();
		float lineSpace = textJson[windowName + "Text"].value("lineSpace", 0.0f);
		auto text = std::make_unique<TextComponent>(*this, zDepth - 0.5f);
		text->setText(message);
		text->setBaseLine(textJson[windowName + "Text"]["x"].get<float>(), textJson[windowName + "Text"]["y"].get<float>());
		text->setFontSize(fontSize);
		text->setTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		text->setLineSpace(lineSpace);
		text->showText();
		addComponent(std::move(text));
		mArrowMoveLength = fontSize + lineSpace;
	}

	//矢印
	auto arrow = std::make_unique<SpriteComponent>(*this);
	arrow->create(spriteJson[windowName + "Arrow"]["filePath"].get<std::string>());
	arrow->setBordarSize(0.0f);
	arrow->setSpriteSize(XMFLOAT2(spriteJson[windowName + "Arrow"]["width"].get<float>(), spriteJson[windowName + "Arrow"]["height"].get<float>()));
	arrow->setPosition(XMFLOAT3(spriteJson[windowName + "Arrow"]["x"].get<float>(), spriteJson[windowName + "Arrow"]["y"].get<float>(), zDepth - 1.0f));
	arrow->setRotation(spriteJson[windowName + "Arrow"]["rotation"].get<float>());
	mArrow = arrow.get();
	addComponent(std::move(arrow));

}

