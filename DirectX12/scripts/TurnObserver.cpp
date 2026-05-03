#include "TurnObserver.h"
#include "DungeonScene.h"
#include "Random.h"
#include "Definition.h"
#include "EnemyComponent.h"
#include "Player.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "input.h"
#include "Game.h"
#include "Graphic.h"
#include "AudioManager.h"
#include "EndWindow.h"

TurnObserver::TurnObserver(DungeonScene& scene)
	: mScene(scene)
{
	mNextTurn = TurnType::PLAYER;
	mTurnType = TurnType::PLAYER;
	mPendingEnemyCount = 0;
	mIsActive = true;
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
		//進行を進めるか
		if (!mIsActive) return;

		//プレイヤーの残り行動回数が0ならば街に帰らせる
		if (mScene.getPlayerActLimit() == 0) {
			auto player = mScene.getPlayer();

			//死亡していたら処理を行わない
			if (player->getHP() > 0) RunOutProcess();
		}


		//敵のランダム湧き
		int random = Random::dist(1, 100);
		if (random <= 10) spawnEnemy();
		random = Random::dist(1, 100);
		if (random <= 10) mScene.spawnResource();
		//ミニマップの更新
		mScene.updateMiniMapPos();
	}


	mTurnType = mNextTurn;
}

void TurnObserver::begin()
{
	mTurnType = TurnType::PLAYER;
	mNextTurn = TurnType::PLAYER;
}

TurnType TurnObserver::getTurnType() const
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

void TurnObserver::stop()
{
	mIsActive = false;
}

void TurnObserver::start()
{
	mIsActive = true;
}


void TurnObserver::spawnEnemy()
{
	int playerIndex[2];
	mScene.getPlayer()->getIndexPos(playerIndex);

	int i = 0; //湧き場がない場合、一定回数のループ後にループを抜ける

	//障害物がない　かつ　プレイヤーから4マス以上9マス以下の範囲でスポーン
	int mapSize = mScene.getMapSize();
	while (i < 50) {
		//スポーンするマスを乱数で決定
		int x = playerIndex[0] + Random::dist(-9, 9);
		int y = playerIndex[1] + Random::dist(-9, 9);


		//障害物がある場合、もう一度乱数を振りなおす
		if (mScene.getTileDataAt(x, y) == TileType::WALL ||
			mScene.getCharacterDataAt(x, y) != CharacterType::EMPTY) {
			i++;
			continue;
		}
		
		//プレイヤーから3マスいないならば、もう一度乱数を振りなおす
		int distance = abs(playerIndex[0] - x) + abs(playerIndex[1] - y);
		if (distance <= 3) {
			i++;
			continue;
		}

		//敵の生成
		mScene.createEnemy("SLIME", static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y));
		break;
	}
}

void TurnObserver::RunOutProcess()
{
	mTurnType = TurnType::END;
	mNextTurn = TurnType::END;

	//エンドウィンドウの生成
	auto endWindow = std::make_unique<EndWindow>(mScene, WindowType::RETURN);
	mScene.addActor(std::move(endWindow));
}

