#pragma once
#include "Actor.h"

class DungeonScene;
class TurnObserver;

class EndWindow :public Actor{
public:
	EndWindow(DungeonScene& scene);
	DECLARE_CLASS_NAME(EndWindow)
	void updateActor() override;
	void inputActor() override;

private:
	void showWindow();
	void startTransitScene();

	int mTimer;
	bool isActive;
	bool isTransitting;
	DungeonScene& mDungeon;
	const TurnObserver& mObserver;
};
