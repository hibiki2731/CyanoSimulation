#include "Enemy.h"
#include "MeshComponent.h"
#include "Game.h"
#include "EnemyComponent.h"
#include "MapManager.h"
#include "Player.h"
#include <fstream>

Enemy::Enemy(Game* game, CharacterType::Type type, float x, float y) : Actor(game)
{
	//敵の位置を設定
	mPosition = XMFLOAT3(x, 0, y);

	//敵のパラメータをjsonファイルから読み込む
	std::ifstream file("assets\\data\\enemyData.json");
	assert(!file.fail());

	//jsonファイルの読み込み
	nlohmann::json enemyData;
	file >> enemyData;

	auto typeName = magic_enum::enum_name(type); //CharacterTypeを文字列に変換

	EnemyParam param;
	//敵のタイプがjsonファイルに存在するか確認
	if (enemyData.contains(typeName)) {
		param.hp = enemyData[typeName]["hp"].get<int>();
		param.power = enemyData[typeName]["power"].get<int>();
		param.defense = enemyData[typeName]["defense"].get<int>();
		param.meshName = magic_enum::enum_cast<MeshName>(enemyData[typeName]["meshName"].get<std::string>()).value();
		param.movePattern = magic_enum::enum_cast<MovePattern>(enemyData[typeName]["movePattern"].get<std::string>()).value();
		param.senseRange = enemyData[typeName].value("senseRange", 0); //senseRangeがない場合はデフォルトで0を使用
	}

	//コンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(this);
	mesh->create(param.meshName);

	auto enemy = std::make_unique<EnemyComponent>(this);
	mEnemy = enemy.get();
	mEnemy->setMesh(mesh.get());
	mEnemy->setEnemyTtype(type);
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

void Enemy::updateActor()
{
}

void Enemy::inputActor()
{
}
