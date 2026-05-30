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
#include "myJson.h"
#include "PointLightComponent.h"
#include "MeshComponent.h"
#include "FireParticleComponent.h"
#include <fstream>
#include <cassert>
#include "Treasure.h"

MapGenerator::MapGenerator(DungeonScene& scene)
	: mScene(scene)
{
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

	loadMap(mScene.getStage());	//マップデータの読み込み
	createTile();	//マップの壁、床の生成
	createCharacter(); //オブジェクトの生成

	//天井の作成
	int mapSize = mScene.getMapSize();
	auto roof = std::make_unique<Object>(mScene, "RockRoof", "ROCK_ROOF", MAPTIPSIZE * (static_cast<float>(mapSize) / 2.0f), MAPTIPSIZE * (static_cast<float>(mapSize) / 2.0f));
	roof->setScale(XMFLOAT3(static_cast<float>(mapSize) * 1.2f, 1, static_cast<float>(mapSize) * 1.2f));
	roof->setPosY(MAPTIPSIZE);
	mScene.addActor(std::move(roof));

}

void MapGenerator::loadMap(Stage stage)
{
	//読み込み用変数
	std::vector<std::vector<int>> tileData;
	std::vector<std::vector<int>> characterData;


	std::ifstream file;
	nlohmann::json json;

	//ファイルの読み込み
	file.open("assets\\data\\mapData.json");
	assert(!file.fail());
	file >> json;

	//ステージに応じたマップデータの読み込み
	nlohmann::json mapJson;
	switch (stage) {
	case Stage::MAP1:
		mapJson = json["stage1"];
		break;
	}

	//マップサイズの読み込み
	const int mapSize = mapJson["mapSize"].get<int>();

	//タイルデータの読み込み
	tileData.resize(mapSize);
	for (int i = 0; i < mapSize; i++) tileData[i].resize(mapSize);
	const nlohmann::json tileJson = mapJson["tileData"];
	//jsonのタイルデータは左上から右に向かって、下の行へと続いているため、読み込む際にy座標を逆転させる
	int x = 0, y = mapSize - 1;
	for (int num : tileJson)
	{
		tileData[x][y] = num;
		x++;
		if (x >= mapSize) {
			x = 0;
			y--;
		}
	}

	//キャラクターデータの読み込み
	characterData.resize(mapSize);
	for (int i = 0; i < mapSize; i++) characterData[i].resize(mapSize);
	const nlohmann::json characterJson = mapJson["characterData"];
	//キャラクターデータも同様にy座標を逆転させて読み込む
	x = 0, y = mapSize - 1;
	for (int num : characterJson)
	{
		characterData[x][y] = num;
		x++;
		if (x >= mapSize) {
			x = 0;
			y--;
		}
	}

	//プレイヤーデータの読み込み
	const XMFLOAT2 playerPos = mapJson["playerPos"].get<XMFLOAT2>();
	mScene.createPlayer(static_cast<float>(MAPTIPSIZE * playerPos.x), static_cast<float>(MAPTIPSIZE * playerPos.y));

	//宝箱データの読み込み
	const nlohmann::json treasureJson = mapJson["treasureData"];
	std::vector<Treasure*> treasures(treasureJson.size());
	int index = 0;
	for (const auto& treasureData : treasureJson)
	{
		const std::vector<int> position = treasureData["position"].get<std::vector<int>>();
		const std::string direction = treasureData["direction"].get<std::string>();
		const std::string category = treasureData["category"].get<std::string>();
		const std::string itemID = treasureData["id"].get<std::string>();
		std::unique_ptr<Treasure> tresure = std::make_unique<Treasure>(mScene, position[0], position[1], direction, category, itemID);
		treasures[index] = tresure.get();
		mScene.addActor(std::move(tresure)); //所有権をGameへ渡す

		index++;
	}


	file.close();

	//シーンにマップデータを渡す
	mScene.setTileData(std::move(tileData));
	mScene.setCharacterData(std::move(characterData));
	mScene.setMapSize(mapSize);
	mScene.setTreasures(std::move(treasures));

}

void MapGenerator::createTile()
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
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mScene, "Tile", tileJson[tileID]["meshID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(rockFloor)); //所有権をGameへ渡す
			}
			else if(category == "RESOURCE"){
				//草の生成
				mScene.createResource(tileJson[tileID]["meshID"].get<std::string>(), tileJson[tileID]["resourceID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y), y * mScene.getMapSize() + x);
				//床の生成
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mScene, "Tile", "ROCK_FLOOR", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(rockFloor)); //所有権をGameへ渡す
			}
			else if (category == "TREASURE") {
				//床の生成
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mScene, "Tile", "ROCK_FLOOR", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(rockFloor)); //所有権をGameへ渡す
			}
			else if (category == "GOAL") {
				//床の生成
				std::unique_ptr<Object> rockFloor = std::make_unique<Object>(mScene, "Tile", "ROCK_FLOOR", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(rockFloor));

				//旗の生成
				std::unique_ptr<Object> flag = std::make_unique<Object>(mScene, "Flag", tileJson[tileID]["meshID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(flag));
			}

			//壁の生成
			//西壁
			if (x == 0) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2); 
				mScene.addActor(std::move(wall));
			} else if(mScene.getTileDataAt(x - 1, y) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			//東壁
			if (x == mapSize - 1) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x + 1, y) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(-XM_PIDIV2);
				mScene.addActor(std::move(wall));
			}
			//北壁
			if (y == mapSize - 1) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x, y + 1) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				wall->setYRot(XM_PI);
				mScene.addActor(std::move(wall));
			}
			//南壁
			if (y == 0) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(wall));
			}
			else if (mScene.getTileDataAt(x, y - 1) == TileType::WALL) {
				auto wall = std::make_unique<Object>(mScene, "Wall", "ROCK_WALL", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
				mScene.addActor(std::move(wall));
			}


		}
	}
}

void MapGenerator::createCharacter()
{
	std::fstream file("assets/data/mapTipData.json");
	nlohmann::json json;
	file >> json;
	auto objectJson = json["object"];

	std::string meshID = "";
	std::string category = "";
	int mapSize = mScene.getMapSize();
	for (int y = 0; y < mapSize; y++){
		for (int x = 0; x < mapSize; x++) {
			meshID = std::to_string(mScene.getCharacterDataAt(x, y));
			category = objectJson[meshID]["category"].get<std::string>();

			if (category == "EMPTY") continue;
			else if (category == "ENEMY") {
				//敵の生成
				mScene.createEnemy(objectJson[meshID]["enemyID"].get<std::string>(), static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
			}

		}
	}
}
