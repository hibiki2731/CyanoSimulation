#include "TurnObserver.h"
#include "DungeonScene.h"
#include "Random.h"
#include "Definition.h"
#include "EnemyComponent.h"
#include "Player.h"

TurnObserver::TurnObserver(DungeonScene& scene)
	: mScene(scene)
{
	mNextTurn = TurnType::PLAYER;
	mTurnType = TurnType::PLAYER;
	mPendingEnemyCount = 0;
}

void TurnObserver::updateTurn()
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

TurnType TurnObserver::getTurnType()
{
	return mTurnType;
}

void TurnObserver::moveToPlayerTurn()
{
	mPendingEnemyCount--;
	if (mPendingEnemyCount == 0) mNextTurn = TurnType::PLAYER;
}

void TurnObserver::moveToEnemyTurn()
{
	mNextTurn = TurnType::ENEMY;
}

void TurnObserver::startEnemyTurn()
{
	//敵配列をプレイヤーに近い順にソート
	mScene.sortEnemiesByDistanceToPlayer();
	mPendingEnemyCount = static_cast<int>(mScene.getEnemyCount());

	for (auto enemy : mScene.getEnemies()) {
		enemy->startAct();
	}
	mScene.updateMiniMapPos();
}

void TurnObserver::spawnEnemy()
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
