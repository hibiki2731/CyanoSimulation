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
enum class Stage;
enum class TurnType;

class MapManager
{
public:
	MapManager(DungeonScene& scene);
	~MapManager() {};

	void begin();
	void end();

	void updateTurn();

	void createMap();
	
	//getter
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

	//未行動敵数
	int mPendingEnemyCount;

};

