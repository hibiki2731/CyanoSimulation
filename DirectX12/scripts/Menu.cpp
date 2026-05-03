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

Menu::Menu(TownScene& scene, const std::string& windowName, float zDepth) 
	: Object(scene, windowName),
	mScene(scene)
{
	mSelectedIndex = 0;
	mMaxIndex = 0;
	mArrow = nullptr;
	scene.pushMenu(this);
	
	addComponentLabel("selectArrow", "SpriteComponent");
	mArrowMoveLength = 60.0f;

	applyComponentLabel();
}

void Menu::inputMenu() {

	if (mArrow) {
		if (isKeyJustPressed(VK_UP)) {
			if (mSelectedIndex <= 0) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex--;
			mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		}

		if (isKeyJustPressed(VK_DOWN)) {
			if (mSelectedIndex >= mMaxIndex - 1) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex++;
			mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
		}
	}
}

void Menu::updateActor()
{
	if (mArrow) {
		//メニューがアクティブでないとき
		if (mScene.getCurrentMenu() != this) {
			mArrow->setPosZ(200.0f);
		}
		else
			mArrow->setPosZ(mArrowDefaultPosition);
	}
}

void Menu::applyComponentLabel()
{
	mArrow = static_cast<SpriteComponent*>(mComponentLabels["selectArrow"].pComponent);
	if (mArrow) {
		mArrowDefaultPosition = mArrow->getPosition().z;
	}
}

