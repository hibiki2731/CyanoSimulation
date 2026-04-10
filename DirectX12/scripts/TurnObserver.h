#pragma once
#include "Actor.h"

enum class TurnType;
class DungeonScene;

class TurnObserver
{
public:
	TurnObserver(DungeonScene& scene);

	void updateTurn();
	void begin();

	//getter
	TurnType getTurnType();

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void startEnemyTurn();

private:
	void endProcess();

	TurnType mNextTurn;
	TurnType mTurnType;
	DungeonScene& mScene;

	//未行動敵数
	int mPendingEnemyCount;

	void spawnEnemy();
};

class EndWindow :public Actor{
public:
	EndWindow(DungeonScene& scene, TurnObserver& observer);
	void updateActor() override;
	void inputActor() override;

private:
	void showWindow();

	int mTimer;
	bool isActive;
	DungeonScene& mDungeon;
	TurnObserver& mObserver;
};
