#include "Enemy.h"
#include "MeshComponent.h"
#include "Game.h"
#include "EnemyComponent.h"
#include "MapManager.h"
#include "json.hpp"
#include "Player.h"
#include <fstream>

Enemy::Enemy(Game& game, MapManager& mapManager, const std::string& enemyID, float x, float y)
	: Actor(game),
	mMapManager(mapManager)
{
	//敵の位置を設定
	mPosition = XMFLOAT3(x, 0, y);

	//敵のパラメータをjsonファイルから読み込む
	std::ifstream file("assets\\data\\enemyData.json");
	assert(!file.fail());

	//jsonファイルの読み込み
	nlohmann::json enemyData;
	file >> enemyData;

	EnemyParam param;
	//敵のタイプがjsonファイルに存在するか確認
	if (enemyData.contains(enemyID)) {
		param.hp = enemyData[enemyID]["hp"].get<int>();
		param.power = enemyData[enemyID]["power"].get<int>();
		param.defense = enemyData[enemyID]["defense"].get<int>();
		param.meshName = enemyData[enemyID]["meshName"].get<std::string>();
		param.movePattern = magic_enum::enum_cast<MovePattern>(enemyData[enemyID]["movePattern"].get<std::string>()).value();
		param.senseRange = enemyData[enemyID].value("senseRange", 0); //senseRangeがない場合はデフォルトで0を使用
	}

	//コンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(param.meshName);

	auto enemy = std::make_unique<EnemyComponent>(*this, mMapManager);
	mEnemy = enemy.get();
	mEnemy->setMesh(mesh.get());
	mEnemy->setDirection(Direction::UP); //上向き
	mEnemy->setDefense(param.defense);
	mEnemy->setPower(param.power);
	mEnemy->setMaxHP(param.hp);
	mEnemy->setMovePattern(param.movePattern);
	//移動パターンがSENSEのときは、senseRangeを設定する
	if (param.movePattern == MovePattern::SENSE) {
		mEnemy->setSenseRange(param.senseRange);
	}

	//所有権をActorに渡す
	addComponent(std::move(mesh));
	addComponent(std::move(enemy));
}

Enemy::~Enemy()
{
	mMapManager.removeEnemy(mEnemy);
	mMapManager.removeActorToMap(this);
}

void Enemy::updateActor()
{
}

void Enemy::inputActor()
{
}
