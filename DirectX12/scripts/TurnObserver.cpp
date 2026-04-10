#include "TurnObserver.h"
#include "DungeonScene.h"
#include "Random.h"
#include "Definition.h"
#include "EnemyComponent.h"
#include "Player.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "input.h"

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
			endProcess();
		}


		//敵のランダム湧き
		int random = Random::dist(1, 100);
		if (random <= 10) spawnEnemy();

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

void TurnObserver::endProcess()
{
	mTurnType = TurnType::END;
	mNextTurn = TurnType::END;

	//エンドウィンドウの生成
	auto endWindow = std::make_unique<EndWindow>(mScene, *this);
	mScene.addActor(std::move(endWindow));
}

EndWindow::EndWindow(DungeonScene& scene, TurnObserver& observer)
	:Actor(scene),
	mDungeon(scene),
	mObserver(observer)
{
	isActive = false;
	mTimer = 0;

}

void EndWindow::updateActor()
{
	if (mDungeon.getPlayer()->getIsActing()) return;

	if (mTimer == 0) showWindow();

	mTimer++;

}

void EndWindow::inputActor()
{
	if (mTimer > 10 && isKeyJustPressed(VK_RETURN)) {
		mDungeon.returnToTown();
	}
}

void EndWindow::showWindow()
{
	std::string structName = "EndWindow";
	auto window = std::make_unique<SpriteComponent>(*this, 30.0f);
	window->loadFileAndCreate(structName);
	window->create("assets/picture/UI2/PNG/Default/panel_brown_damaged.png");
#ifdef _DEBUG
	window->activateControll(structName);
#endif
	addComponent(std::move(window));

	structName = "EndWindowText";
	auto text = std::make_unique<TextComponent>(*this, 29.0f);
	text->loadFileAndCreate(structName);
	text->setTextColor(D2D1::ColorF::Red);
	std::wstring message = L" ";
	if (mObserver.getTurnType() == TurnType::END) {
		message = L"体力が尽きました\n";
	}
	text->setText(message);
	text->showText();
#ifdef _DEBUG
	text->activateControll(structName);
#endif
	addComponent(std::move(text));

}
