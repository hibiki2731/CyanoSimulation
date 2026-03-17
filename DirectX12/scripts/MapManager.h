#pragma once
#include <vector>
#include <memory>
#include "Definition.h"
#include "Random.h"

class Game;
class Player;
class SceneManager;

enum class Stage {
	MAP1,
};
struct TileType {
	enum Type {
		WALL = 0,
		FLOOR = 1,
		GRASS = 2,
	};
};

struct CharacterType {
	enum Type {
		EMPTY = 0,
		PLAYER = 1,
		SLIME = 2,
		NURIKABE = 3,
	};
};

class MapManager
{
public:
	MapManager(Game* game);
	~MapManager() {};

	void updateTurn();
	void sceneProcess();

	void createMap();
	
	//setter
	void setStage(Stage stage);
	void setMapDataAt(int x, int y, int data);
	void setMapDataAt(int index, int data);
	void setObjectDataAt(int x, int y, int data);
	void setObjectDataAt(int index, int data);

	//getter
	int getMapSize();
	int getMapDataAt(int x, int y);
	int getMapDataAt(int index);
	int getObjectDataAt(int x, int y);
	int getObjectDataAt(int index);
	Player* getPlayer();
	TurnType getTurnType();

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void clearMap();

private:
	void loadMap(Stage stage);
	void createWall();
	void createObject();
	void spawnEnemy();

	TurnType mNextTurn;
	TurnType mTurnType;
	std::vector<std::vector<int>> mMapData; //[x][y]
	std::vector<std::vector<int>> mObjectData; //[x][y]
	int mMapSize;
	Stage mStage;
	Game* mGame;
	SceneManager* mSceneManager;

	//未行動敵数
	int mPendingEnemyCount;

	//参照用プレイヤー
	Player* mPlayer;

	//シーン制御
	bool isMap;
};

