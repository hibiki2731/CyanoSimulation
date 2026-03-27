#include "MapManager.h"
#include "Game.h"
#include "Object.h"
#include "Definition.h"
#include "Enemy.h"
#include "Player.h"
#include "Grass.h"
#include "MessageWindow.h"
#include "AudioManager.h"
#include "SceneManager.h"
#include "MiniMap.h"
#include "json.hpp"
#include <fstream>
#include <cassert>

MapManager::MapManager(Game* game)
{
	mMapData.clear();
	mMapSize = 1;
	mGame = game;
	mStage = Stage::MAP1;
	mNextTurn = TurnType::PLAYER;
	mTurnType = TurnType::PLAYER;
	mPendingEnemyCount = 0;
	mSceneManager = mGame->getSceneManager();
	isMap = false;

}

void MapManager::updateTurn()
{
	//MAPシーン中の処理
	if (isMap) {
		//エネミーターン時に敵が全滅していたらプレイヤーターンへ
		if (mTurnType == TurnType::ENEMY && mGame->getEnemies().size() == 0) {
			mNextTurn = TurnType::PLAYER;
		}

		//プレイヤーターン→エネミーターンへの移行時
		if (mNextTurn == TurnType::ENEMY && mTurnType == TurnType::PLAYER) {
			//初期化
			mPendingEnemyCount = static_cast<int>(mGame->getEnemies().size()); //待機敵数をリセット
			mGame->activateEnemies();
			mMiniMap->updatePosition();
		}

		//エネミーターン→プレイヤーターンへの移行時
		if (mNextTurn == TurnType::PLAYER && mTurnType == TurnType::ENEMY) {
			//プレイヤーの残り行動回数が0ならば街に帰らせる
			if (mPlayer->getActionLimit() == 0) {
				mSceneManager->transitToTown();
			}


			//敵のランダム湧き
			int random = Random::dist(1, 100);
			if (random <= 10) spawnEnemy();

			//ミニマップの更新
			mMiniMap->updatePosition();
		}


		mTurnType = mNextTurn;
	}
}

void MapManager::sceneProcess() {
	//マップシーンに切り替わった際の処理
	if (!isMap && mSceneManager->getCurrentScene() == SceneType::MAP) {
		isMap = true;
		mGame->getAudioManager()->playBGM("BGM_DUNGEON2");

		createMap();

		std::unique_ptr<MessageWindow> messageWindow = std::make_unique<MessageWindow>(mGame);
		mGame->addActor(std::move(messageWindow));

		//ミニマップの作成
		auto minimap = std::make_unique<MiniMap>(mGame);
		mMiniMap = minimap.get();
		mGame->addActor(std::move(minimap));
		mMiniMap->updatePosition();

	}

	//マップシーンから他のシーンに切り替わった際の処理
	if (isMap && mSceneManager->getCurrentScene() != SceneType::MAP) {
		isMap = false;
		mPlayer = nullptr;
		mMiniMap = nullptr;
	}
}

void MapManager::createMap()
{

	loadMap(mStage);	//マップデータの読み込み
	createWall();	//マップの壁、床の生成
	createObject(); //オブジェクトの生成

}

void MapManager::setStage(Stage stage)
{
	mStage = stage;
}

void MapManager::setMapDataAt(int x, int y, int data)
{
	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mMapData[x][y] = data;
}

void MapManager::setMapDataAt(int index, int data)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mMapData[x][y] = data;
}

void MapManager::setObjectDataAt(int x, int y, int data)
{
	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mObjectData[x][y] = data;
}

void MapManager::setObjectDataAt(int index, int data)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mObjectData[x][y] = data;
}

int MapManager::getMapSize()
{
	return mMapSize;
}

int MapManager::getMapDataAt(int x, int y)
{
	//配列範囲外なら壁を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return TileType::WALL;

	return mMapData[x][y];
}

int MapManager::getMapDataAt(int index)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら壁を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return TileType::WALL;
	return mMapData[x][y];
}

int MapManager::getObjectDataAt(int x, int y)
{
	//配列範囲外なら空を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return CharacterType::EMPTY;

	return mObjectData[x][y];
}

int MapManager::getObjectDataAt(int index)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら空を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return CharacterType::EMPTY;

	return mObjectData[x][y];
}
const std::string& MapManager::getResourceID(int index)
{
	return mResourceIDs[index];
}

const std::string& MapManager::getResourceID(int x, int y) {
	return mResourceIDs[y + mMapSize + x];
}
Player* MapManager::getPlayer()
{
	//シーンがMAP以外の場合はnullptrを返す
	if (mGame->getSceneManager()->getCurrentScene() != SceneType::MAP) return nullptr;

	return mPlayer;
}

MiniMap* MapManager::getMiniMap()
{
	//シーンがMAP以外の場合はnullptrを返す
	if (mGame->getSceneManager()->getCurrentScene() != SceneType::MAP) return nullptr;

	return mMiniMap;

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

void MapManager::clearMap()
{
	mMapData.clear();
	mObjectData.clear();
}

void MapManager::loadMap(Stage stage)
{
	std::ifstream file;

	//ファイルの読み込み
	switch (stage) {
	case Stage::MAP1:
		file.open("assets\\mapdata\\stage1.txt");
		break;
	}

	assert(!file.fail());
	file >> mMapSize;

	//マップデータの読み込み
	mMapData.resize(mMapSize);
	for (int i = 0; i < mMapSize; i++) mMapData[i].resize(mMapSize);

	for (int y = mMapSize - 1; y >= 0; y--)
	{
		for (int x = 0; x < mMapSize; x++)
		{
			int tileNum = 0;
			file >> tileNum;
			mMapData[x][y] = tileNum;
		}
	}
	//オブジェクトデータの読み込み
	mObjectData.resize(mMapSize);
	for (int i = 0; i < mMapSize; i++) mObjectData[i].resize(mMapSize);

	for (int y = mMapSize - 1; y >= 0; y--)
	{
		for (int x = 0; x < mMapSize; x++)
		{
			int objectNum = 0;
			file >> objectNum;
			mObjectData[x][y] = objectNum;
		}
	}

	file.close();

}

void MapManager::createWall()
{
	std::fstream file("assets/data/mapTipData.json");
	nlohmann::json json;
	file >> json;
	auto tileJson = json["tile"];

	for (int y = 0; y < mMapSize; y++)
	{
		for (int x = 0; x < mMapSize; x++)
		{
			std::string tileID = std::to_string(mMapData[x][y]);
			std::string category = tileJson[tileID]["category"].get<std::string>();

			if (category == "WALL") continue; //壁の中
			else if(category == "FLOOR") {
				//床の生成
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mGame, tileJson[tileID]["meshID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mGame->addActor(std::move(rockFloor)); //所有権をGameへ渡す
			}
			else if(category == "RESOURCE"){
				//草の生成
				std::unique_ptr<Resource> grass = std::make_unique<Resource>(mGame, tileJson[tileID]["meshID"].get<std::string>(), tileJson[tileID]["resourceID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mResourceIDs[y * mMapSize + x] = tileJson[tileID]["resourceID"].get<std::string>();
				mGame->addActor(std::move(grass)); //所有権をGameへ渡す
			}

			//壁の生成
			//西壁
			if (x == 0) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2); 
				mGame->addActor(std::move(wall));
			} else if(mMapData[x - 1][y] == TileType::WALL) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2);
				mGame->addActor(std::move(wall));
			}
			//東壁
			if (x == mMapSize - 1) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mGame->addActor(std::move(wall));
			}
			else if (mMapData[x + 1][y] == TileType::WALL) {
				auto wall = std::make_unique<Object>(mGame,"ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mGame->addActor(std::move(wall));
			}
			//北壁
			if (y == mMapSize - 1) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mGame->addActor(std::move(wall));
			}
			else if (mMapData[x][y + 1] == TileType::WALL) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mGame->addActor(std::move(wall));
			}
			//南壁
			if (y == 0) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mGame->addActor(std::move(wall));
			}
			else if (mMapData[x][y - 1] == TileType::WALL) {
				auto wall = std::make_unique<Object>(mGame, "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mGame->addActor(std::move(wall));
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
	for (int y = 0; y < mMapSize; y++){
		for (int x = 0; x < mMapSize; x++) {
			objectID = std::to_string(mObjectData[x][y]);
			category = objectJson[objectID]["category"].get<std::string>();

			if (category == "EMPTY") continue;
			else if (category == "PLAYER") {
				//プレイヤー生成
				std::unique_ptr player = std::make_unique<Player>(mGame, static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mPlayer = player.get();
				mGame->addActor(std::move(player)); //所有権をGameへ渡す
			}
			else if (category == "ENEMY") {
				//敵の生成
				std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(mGame, objectJson[objectID]["enemyID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mGame->addActor(std::move(enemy)); //所有権をGameへ渡す
			}

		}
	}
}

void MapManager::spawnEnemy()
{
	int playerIndex[2];
	mPlayer->getIndexPos(playerIndex);

	int i = 0; //湧き場がない場合、一定回数のループ後にループを抜ける

	//障害物がない　かつ　プレイヤーから3マス離れているところにスポーン
	while (i < 10) {
		//スポーンするマスを乱数で決定
		int x = Random::dist(0, mMapSize - 1);
		int y = Random::dist(0, mMapSize - 1);

		//障害物がある場合、もう一度乱数を振りなおす
		if (mMapData[x][y] == TileType::WALL) continue;
		if (mObjectData[x][y] != CharacterType::EMPTY) continue;
		
		//プレイヤーから3マスいないならば、もう一度乱数を振りなおす
		int distance = abs(playerIndex[0] - x) + abs(playerIndex[1] - y);
		if (distance <= 3) continue;

		//敵の生成
		std::unique_ptr<Enemy> slime = std::make_unique<Enemy>(mGame, "SLIME", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
		mGame->addActor(std::move(slime)); //所有権をGameへ渡す
		break;

		i++;
	}
}
