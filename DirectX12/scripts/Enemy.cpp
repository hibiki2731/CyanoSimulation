#include "Enemy.h"
#include "MeshComponent.h"
#include "Game.h"
#include "EnemyComponent.h"
#include "Player.h"
#include "DungeonScene.h"
#include "ItemManager.h"
#include <fstream>

Enemy::Enemy(DungeonScene& scene, const std::string& enemyID, float x, float y)
	: Actor(scene)
{
	//敵の位置を設定
	mPosition = XMFLOAT3(x, 0, y);

	nlohmann::json& enemyData = scene.getGame().getAssetManager().getEnemyJson();

	//コンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(enemyData[enemyID]["meshID"].get<std::string>());

	auto enemy = std::make_unique<EnemyComponent>(*this, scene);
	mEnemy = enemy.get();
	mEnemy->setMesh(mesh.get());
	mEnemy->setDirection(Direction::UP); //上向き
	mEnemy->setDefense(enemyData[enemyID]["defense"].get<int>());
	mEnemy->setPower(enemyData[enemyID]["power"].get<int>());
	mEnemy->setMaxHP(enemyData[enemyID]["hp"].get<int>());
	int dropMoney = enemyData[enemyID].value("dropMoney", 0) + scene.getGame().getItemManager().getResourceData("GOLD").yield;
	mEnemy->setDropMoney(dropMoney);
	mEnemy->setName(enemyData[enemyID]["name"].get<std::string>());
	MovePattern pattern = magic_enum::enum_cast<MovePattern>(enemyData[enemyID]["movePattern"].get<std::string>()).value();
	mEnemy->setMovePattern(pattern);
	//移動パターンがSENSEのときは、senseRangeを設定する
	if (pattern == MovePattern::SENSE) {
		mEnemy->setSenseRange(enemyData[enemyID].value("senseRange", 0));
	}

	//所有権をActorに渡す
	addComponent(std::move(mesh));
	addComponent(std::move(enemy));
}

