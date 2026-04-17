#include "EndWindow.h"
#include "DungeonScene.h"
#include "Player.h"
#include "TurnObserver.h"
#include "Definition.h"
#include "input.h"
#include "AudioManager.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "GUIDebugger.h"
#include "Graphic.h"
#include "Game.h"


EndWindow::EndWindow(DungeonScene& scene)
	:Actor(scene),
	mDungeon(scene),
	mObserver(scene.getTurnObserver())
{
	isActive = false;
	mTimer = 0;
	isTransitting = false;

}

void EndWindow::updateActor()
{
	if (mDungeon.getPlayer()->getIsActing()) return;

	if (mTimer == 0) showWindow();

	if (isTransitting && mScene.getGame().getGraphic().isFinishedFade()) {
		if (mObserver.getTurnType() == TurnType::END)
			mDungeon.returnToTown();
		else
			mDungeon.transitToGameOver();
	}

	mTimer++;

}

void EndWindow::inputActor()
{
	if (mTimer > 10 && isKeyJustPressed(VK_RETURN)) {
		startTransitScene();
	}
}

void EndWindow::showWindow()
{
	std::string structName = "EndWindow";
	auto window = std::make_unique<SpriteComponent>(*this, 30.0f);
	window->loadFileAndCreate(structName);
	window->create("assets/picture/UI2/PNG/Default/panel_brown_damaged.png");
#ifdef _DEBUG
	window->activateControll(structName);
#endif
	addComponent(std::move(window));

	structName = "EndWindowText";
	auto text = std::make_unique<TextComponent>(*this, 29.0f);
	text->loadFileAndCreate(structName);
	text->alignCenter(450.0f);
	std::wstring message = L" ";
	if (mObserver.getTurnType() == TurnType::END) {
		message = L"体力が尽きました\n街へ帰還します\n";
	} 
	else {
		text->setTextColor(D2D1::ColorF::DarkRed);
		text->setPosition(text->getPosX(), text->getPosY() + 10.0f);
		message = L"あなたは死にました\n";
	}
	text->setText(message);
#ifdef _DEBUG
	text->activateControll(structName);
#endif
	addComponent(std::move(text));

	mScene.getGame().getAudioManager().finishAllSounds();
	mScene.getGame().getAudioManager().playSE("DUNGEON_END");

}

void EndWindow::startTransitScene()
{
	isTransitting = true;
	mScene.getGame().getGraphic().startFadeOut(1.0f);
	if (mObserver.getTurnType() == TurnType::END) 
		mScene.getGame().getAudioManager().playSE("ESCAPE");
}
