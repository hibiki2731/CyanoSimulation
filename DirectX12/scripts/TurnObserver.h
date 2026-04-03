#pragma once

enum class TurnType;
class DungeonScene;

class TurnObserver
{
public:
	TurnObserver(DungeonScene& scene);

	void updateTurn();

	//getter
	TurnType getTurnType();

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void startEnemyTurn();

private:
	TurnType mNextTurn;
	TurnType mTurnType;
	DungeonScene& mScene;

	//未行動敵数
	int mPendingEnemyCount;

	void spawnEnemy();
};

