#pragma once
#include "Object.h"

class DungeonScene;
class TurnObserver;
class TextComponent;

enum class WindowType{
	DEAD,
	RETURN,
	GOAL
};

class EndWindow :public Object{
public:
	EndWindow(DungeonScene& scene, const WindowType& type);
	DECLARE_CLASS_NAME(EndWindow)
	void updateActor() override;
	void inputActor() override;

	void applyComponentLabel() override;

private:
	void showWindow();
	void startTransitScene();

	int mTimer;
	bool isActive;
	bool isTransitting;
	DungeonScene& mDungeon;
	const TurnObserver& mObserver;
	const WindowType mType;

	//各テキスト
	TextComponent* mDeadText;
	TextComponent* mRunOutText;
	TextComponent* mGoalText;
};
