#include "MapManager.h"
#include "Game.h"
#include "DungeonScene.h"
#include "Object.h"
#include "Definition.h"
#include "Enemy.h"
#include "Player.h"
#include "MiniMap.h"
#include "EnemyComponent.h"
#include "AudioManager.h"
#include "json.hpp"
#include <fstream>
#include <cassert>

MapManager::MapManager(DungeonScene& scene)
	: mScene(scene)
{
	mStage = Stage::MAP1;
	mNextTurn = TurnType::PLAYER;
	mTurnType = TurnType::PLAYER;
	mPendingEnemyCount = 0;

}

void MapManager::begin()
{
	mScene.getGame().getAudioManager()->playBGM("BGM_DUNGEON2");

	createMap();

}

void MapManager::end()
{
}

void MapManager::updateTurn()
{
	//エネミーターン時に敵が全滅していたらプレイヤーターンへ
	if (mTurnType == TurnType::ENEMY &&  mScene.getEnemyCount() == 0) {
		mNextTurn = TurnType::PLAYER;
	}

	//プレイヤーターン→エネミーターンへの移行時
	if (mNextTurn == TurnType::ENEMY && mTurnType == TurnType::PLAYER) {
		//初期化
		startEnemyTurn();
	}

	//エネミーターン→プレイヤーターンへの移行時
	if (mNextTurn == TurnType::PLAYER && mTurnType == TurnType::ENEMY) {
		//プレイヤーの残り行動回数が0ならば街に帰らせる
		if (mScene.getPlayerActLimit() == 0) {
			mScene.returnToTown();
		}


		//敵のランダム湧き
		int random = Random::dist(1, 100);
		if (random <= 10) spawnEnemy();

		//ミニマップの更新
		mScene.updateMiniMapPos();
	}


	mTurnType = mNextTurn;
}

void MapManager::createMap()
{

	loadMap(mStage);	//マップデータの読み込み
	createWall();	//マップの壁、床の生成
	createObject(); //オブジェクトの生成

}

TurnType MapManager::getTurnType()
{
	return mTurnType;
}

void MapManager::moveToPlayerTurn()
{
	mPendingEnemyCount--;
	if (mPendingEnemyCount == 0) mNextTurn = TurnType::PLAYER;

}

void MapManager::moveToEnemyTurn()
{
	mNextTurn = TurnType::ENEMY;
	
}

void MapManager::startEnemyTurn()
{
	//敵配列をプレイヤーに近い順にソート
	mScene.sortEnemiesByDistanceToPlayer();
	mPendingEnemyCount = static_cast<int>(mScene.getEnemyCount());

	for (auto enemy : mScene.getEnemies()) {
		enemy->startAct();
	}
	mScene.updateMiniMapPos();
}

void MapManager::loadMap(Stage stage)
{
	//読み込み用変数
	int mapSize;
	std::vector<std::vector<int>> tileData;
	std::vector<std::vector<int>> characterData;


	std::ifstream file;

	//ファイルの読み込み
	switch (stage) {
	case Stage::MAP1:
		file.open("assets\\mapdata\\stage1.txt");
		break;
	}

	assert(!file.fail());
	file >> mapSize;

	//マップデータの読み込み
	tileData.resize(mapSize);
	for (int i = 0; i < mapSize; i++) tileData[i].resize(mapSize);

	for (int y = mapSize - 1; y >= 0; y--)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int tileNum = 0;
			file >> tileNum;
			tileData[x][y] = tileNum;
		}
	}
	//オブジェクトデータの読み込み
	characterData.resize(mapSize);
	for (int i = 0; i < mapSize; i++) characterData[i].resize(mapSize);

	for (int y = mapSize - 1; y >= 0; y--)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int objectNum = 0;
			file >> objectNum;
			characterData[x][y] = objectNum;
		}
	}

	file.close();

	//シーンにマップデータを渡す
	mScene.setTileData(std::move(tileData));
	mScene.setCharacterData(std::move(characterData));
	mScene.setMapSize(mapSize);

}

void MapManager::createWall()
{
	std::fstream file("assets/data/mapTipData.json");
	nlohmann::json json;
	file >> json;
	auto tileJson = json["tile"];
	int mapSize = mScene.getMapSize();

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			std::string tileID = std::to_string(mScene.getTileDataAt(x, y));
			std::string category = tileJson[tileID]["category"].get<std::string>();

			if (category == "WALL") continue; //壁の中
			else if(category == "FLOOR") {
				//床の生成
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mScene, tileJson[tileID]["meshID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(rockFloor)); //所有権をGameへ渡す
			}
			else if(category == "RESOURCE"){
				//草の生成
				mScene.createResource(tileJson[tileID]["meshID"].get<std::string>(), tileJson[tileID]["resourceID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y), y * mScene.getMapSize() + x);
			}

			//壁の生成
			//西壁
			if (x == 0) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2); 
				mScene.addActor(std::move(wall));
			} else if(mScene.getTileDataAt(x - 1, y) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			//東壁
			if (x == mapSize - 1) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x + 1, y) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			//北壁
			if (y == mapSize - 1) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x, y + 1) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mScene.addActor(std::move(wall));
			}
			//南壁
			if (y == 0) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x, y - 1) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(wall));
			}


		}
	}
}

void MapManager::createObject()
{
	std::fstream file("assets/data/mapTipData.json");
	nlohmann::json json;
	file >> json;
	auto objectJson = json["object"];

	std::string objectID = "";
	std::string category = "";
	int mapSize = mScene.getMapSize();
	for (int y = 0; y < mapSize; y++){
		for (int x = 0; x < mapSize; x++) {
			objectID = std::to_string(mScene.getCharacterDataAt(x, y));
			category = objectJson[objectID]["category"].get<std::string>();

			if (category == "EMPTY") continue;
			else if (category == "PLAYER") {
				//プレイヤー生成
				mScene.createPlayer(static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
			}
			else if (category == "ENEMY") {
				//敵の生成
				mScene.createEnemy(objectJson[objectID]["enemyID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
			}

		}
	}
}

void MapManager::spawnEnemy()
{
	int playerIndex[2];
	mScene.getPlayer()->getIndexPos(playerIndex);

	int i = 0; //湧き場がない場合、一定回数のループ後にループを抜ける

	//障害物がない　かつ　プレイヤーから3マス離れているところにスポーン
	int mapSize = mScene.getMapSize();
	while (i < 10) {
		//スポーンするマスを乱数で決定
		int x = Random::dist(0, mapSize - 1);
		int y = Random::dist(0, mapSize - 1);

		//障害物がある場合、もう一度乱数を振りなおす
		if (mScene.getTileDataAt(x, y) == TileType::WALL) continue;
		if (mScene.getCharacterDataAt(x, y) != CharacterType::EMPTY) continue;
		
		//プレイヤーから3マスいないならば、もう一度乱数を振りなおす
		int distance = abs(playerIndex[0] - x) + abs(playerIndex[1] - y);
		if (distance <= 3) continue;

		//敵の生成
		mScene.createEnemy("SLIME", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
		break;

		i++;
	}
}
