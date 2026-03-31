#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Definition.h"
#include "Random.h"

class Game;
class Player;
class SceneManager;
class MiniMap;
class EnemyComponent;
class Resource;

enum class Stage {
	MAP1,
};
struct TileType {
	enum Type {
		WALL = 0,
		FLOOR = 1,
		RESOURCE = 2,
	};
};

struct CharacterType {
	enum Type {
		EMPTY = 0,
		PLAYER = -1,
		ENEMY = 1,
	};
};

class MapManager
{
public:
	MapManager(Game& game, SceneManager* sceneManager);
	~MapManager() {};

	void begin();
	void end();

	void updateTurn();

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
	const std::string& getResourceID(int index);
	const std::string& getResourceID(int x, int y);
	Player* getPlayer();
	MiniMap* getMiniMap();
	TurnType getTurnType();

	//ターン制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();
	void clearMap();
	void startEnemyTurn();


	void addActorToMap(class Actor* actor);
	void removeActorToMap(class Actor* actor);

	//エネミー配列の制御
	void addEnemy(EnemyComponent* enemy);
	void removeEnemy(EnemyComponent* enemy);
	EnemyComponent* getEnemyFromIndexPos(int index);
	EnemyComponent* getEnemyFromIndexPos(int x, int y);

private:
	void loadMap(Stage stage);
	void createWall();
	void createObject();
	void spawnEnemy();


	TurnType mNextTurn;
	TurnType mTurnType;
	std::vector<std::vector<int>> mMapData; //[x][y]
	std::vector<std::vector<int>> mObjectData; //[x][y]
	std::unordered_map<int, std::string> mResourceIDs;
	int mMapSize;
	Stage mStage;
	Game& mGame;
	SceneManager* mSceneManager;

	//未行動敵数
	int mPendingEnemyCount;

	//参照用プレイヤー
	Player* mPlayer;

	//シーン制御
	bool isMap;

	//敵配列
	std::vector<EnemyComponent*> mEnemies;

	//ミニマップ
	MiniMap* mMiniMap;

	//生成したアクター
	std::vector<class Actor*> mActors;
};

