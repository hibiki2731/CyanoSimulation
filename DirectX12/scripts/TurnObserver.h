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
	TurnType getTurnType() const;

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void startEnemyTurn();

private:
	void RunOutProcess();

	TurnType mNextTurn;
	TurnType mTurnType;
	DungeonScene& mScene;

	//未行動敵数
	int mPendingEnemyCount;

	void spawnEnemy();
};

