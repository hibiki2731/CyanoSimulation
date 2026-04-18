#include "MapGenerator.h"
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

MapGenerator::MapGenerator(DungeonScene& scene)
	: mScene(scene)
{
	mStage = Stage::MAP1;

}

void MapGenerator::begin()
{

	createMap();

}

void MapGenerator::end()
{
}

void MapGenerator::createMap()
{

	loadMap(mStage);	//マップデータの読み込み
	createWall();	//マップの壁、床の生成
	createObject(); //オブジェクトの生成

	//天井の作成
	int mapSize = mScene.getMapSize();
	auto roof = std::make_unique<Object>(mScene, "ROCK_ROOF", MAPTIPSIZE * (static_cast<float>(mapSize) / 2.0f), MAPTIPSIZE * (static_cast<float>(mapSize) / 2.0f));
	roof->setScale(XMFLOAT3(static_cast<float>(mapSize) * 1.2f, 1, static_cast<float>(mapSize) * 1.2f));
	roof->setPosY(MAPTIPSIZE);
	mScene.addActor(std::move(roof));
}

void MapGenerator::loadMap(Stage stage)
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

void MapGenerator::createWall()
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

void MapGenerator::createObject()
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
