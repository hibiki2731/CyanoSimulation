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


EndWindow::EndWindow(DungeonScene& scene, const WindowType& type)
	:Object(scene, "EndWindow"),
	mDungeon(scene),
	mObserver(scene.getTurnObserver()),
	mType(type)
{
	isActive = false;
	mTimer = 0;
	isTransitting = false;

	addComponentLabel("deadText", "TextComponent");
	addComponentLabel("runOutText", "TextComponent");
	addComponentLabel("goalText", "TextComponent");

	applyComponentLabel();

	showWindow();
}

void EndWindow::updateActor()
{
	if (mDungeon.getPlayer()->getIsActing()) return;

	if (isTransitting && mScene.getGame().getGraphic().isFinishedFade()) {
		if (mType == WindowType::RETURN || mType == WindowType::GOAL)
			mDungeon.returnToTown();
		else
			mDungeon.transitToGameOver();
	}

	mTimer++;

}

void EndWindow::inputActor()
{
	if (mTimer > 10 && (isKeyJustPressed(VK_RETURN) || isKeyJustPressed('K'))) {
		startTransitScene();
	}
}

void EndWindow::applyComponentLabel()
{
	mDeadText = static_cast<TextComponent*>(mComponentLabels["deadText"].pComponent);
	mRunOutText = static_cast<TextComponent*>(mComponentLabels["runOutText"].pComponent);
	mGoalText = static_cast<TextComponent*>(mComponentLabels["goalText"].pComponent);

	if (mDeadText) mDeadText->alignCenter(450.0f);
	if (mRunOutText) mRunOutText->alignCenter(450.0f);
	if (mGoalText) mGoalText->alignCenter(500.0f);

	switch (mType) {
	case WindowType::DEAD:
		if (mRunOutText) mRunOutText->setPosZ(101.0f);
		if (mGoalText) mGoalText->setPosZ(101.0f);
		break;
	case WindowType::RETURN:
		if (mDeadText) mDeadText->setPosZ(101.0f);
		if (mGoalText) mGoalText->setPosZ(101.0f);
		break;
	case WindowType::GOAL:
		if (mDeadText) mDeadText->setPosZ(101.0f);
		if (mRunOutText) mRunOutText->setPosZ(101.0f);
	}
}

void EndWindow::showWindow()
{

	switch (mType) {
	case WindowType::RETURN:
		mScene.getGame().getAudioManager().playSE("DUNGEON_END");
		break;
	case WindowType::DEAD:
	//BGMの停止
		mScene.getGame().getAudioManager().finishAllSounds();
		mScene.getGame().getAudioManager().playSE("DUNGEON_END");
		break;
	case WindowType::GOAL:
		break;
	}

}

void EndWindow::startTransitScene()
{
	isTransitting = true;
	mScene.getGame().getGraphic().startFadeOut(1.0f);
	if (mType == WindowType::RETURN) 
		mScene.getGame().getAudioManager().playSE("ESCAPE");
}
