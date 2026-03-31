#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Random.h"

class Game;
class Player;
class SceneManager;
class MiniMap;
class EnemyComponent;
class Resource;
class DungeonScene;

class MapManager
{
public:
	MapManager(Game& game, DungeonScene& scene);
	~MapManager() {};

	void begin();
	void end();

	void updateTurn();

	void createMap();
	
	//getter
	MiniMap* getMiniMap();
	TurnType getTurnType();

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void startEnemyTurn();


private:
	void loadMap(Stage stage);
	void createWall();
	void createObject();
	void spawnEnemy();

	//ダンジョンシーン	
	DungeonScene& mScene;

	TurnType mNextTurn;
	TurnType mTurnType;
	Stage mStage;
	Game& mGame;

	//未行動敵数
	int mPendingEnemyCount;

	//ミニマップ
	MiniMap* mMiniMap;

};

