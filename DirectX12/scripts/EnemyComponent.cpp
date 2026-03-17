#include "EnemyComponent.h"
#include "Actor.h"
#include "Game.h"
#include "Player.h"
#include "Random.h"

EnemyComponent::EnemyComponent(Actor* owner, int updateOrder) : CharacterComponent(owner, updateOrder)
{
	mOwner->getGame()->addEnemy(this);
	mMesh = nullptr;

	mFlashTimer = 0.0f;
	mFlashDuration = 0.3f;	//ダメージを受けたときの点滅時間

	isMoving = false;
	mMoveSpeed =5.0f;

	mEnemyType = CharacterType::EMPTY;
	mState = MovePattern::RANDOM;
	isActive = false;
	mTargetPos = mOwner->getPosition();
	mDistPlayer = 10000000;

	mIndexPos[0] = static_cast<int>(std::round(mOwner->getPosition().x / MAPTIPSIZE));
	mIndexPos[1] = static_cast<int>(std::round(mOwner->getPosition().z / MAPTIPSIZE));
}

void EnemyComponent::inputComponent()
{
	CharacterComponent::inputComponent();
}

void EnemyComponent::updateComponent()
{

	//点滅処理の更新
	updateFlash();

	switch (mMapManager->getTurnType()) {
		//プレイヤーターン時の動作
	case TurnType::PLAYER:
		break;
		//エネミーターン時の動作
	case TurnType::ENEMY:


		//移動
		if (isMoving) {
			//移動する差分の計算
			XMFLOAT3 diffPos = mTargetPos - mOwner->getPosition();
			float moveLength = deltaTime * mMoveSpeed;

			//位置の更新
			if (fabsf(diffPos.x) > moveLength || fabsf(diffPos.y) > moveLength || fabsf(diffPos.z) > moveLength) {
				mOwner->setPosition(mOwner->getPosition() + Math::normalize(diffPos) * moveLength);
			}
			//移動終了時の処理
			else
			{
				mOwner->setPosition(mTargetPos);

				//ターン経過
				isMoving = false;
				finishAct();
			}
		}

		break;
	}

	//死亡したらActor配列から除去
	if (!isAlive) {
		mOwner->setState(Actor::State::Dead);
		finishAct();
	}

}	

void EnemyComponent::endProccess()
{
	CharacterComponent::endProccess();
	mMapManager->setObjectDataAt(mIndexPos[0], mIndexPos[1], CharacterType::EMPTY); //自分のいるindex座標を空に
	mOwner->getGame()->removeEnemy(this);
}

void EnemyComponent::updateActiveProcess()
{
	//activeな時にのみ行う
	if (isActive)
	{
		//プレイヤーとの距離を計算
		int playerIndex[2];
		mMapManager->getPlayer()->getIndexPos(playerIndex);
		mDistPlayer = abs(playerIndex[0] - mIndexPos[0]) + abs(playerIndex[1] - mIndexPos[1]);

		attack();
		move();

		//プレイヤーとの距離を計算
		mDistPlayer = abs(playerIndex[0] - mIndexPos[0]) + abs(playerIndex[1] - mIndexPos[1]);
	}

}

void EnemyComponent::setMesh(MeshComponent* mesh)
{
	mMesh = mesh;
}

void EnemyComponent::setEnemyTtype(int type)
{
	mEnemyType = type;
}

void EnemyComponent::setMovePattern(MovePattern state)
{
	mState = state;
}

void EnemyComponent::setSenseRange(int range)
{
	mSenseRange = range;
}

void EnemyComponent::activate()
{
	isActive = true;
}

void EnemyComponent::startFlash()
{
	mFlashTimer = mFlashDuration;
}

XMFLOAT3 EnemyComponent::getPosition()
{
	return mOwner->getPosition();
}

int EnemyComponent::getDist()
{
	return mDistPlayer;
}

void EnemyComponent::updateFlash()
{
	if (mMesh == nullptr) return;	//メッシュがセットされていない場合は処理しない

	//点滅処理
	if(mFlashTimer > 0.0f) {
		mFlashTimer -= deltaTime;
		float intensity = max(0.0f, mFlashTimer / mFlashDuration);
		mMesh->updateFlashIntensity(intensity * intensity);
	}
}

void EnemyComponent::move()
{
	//移動時は実行できない
	if (isMoving) return;
	//エネミーターン時のみ実行
	if (mMapManager->getTurnType() == TurnType::PLAYER) return;
	//行動済みならスキップ
	if (!isActive) return;

	isMoving = true;

	int targetIndexPos[2];
	//移動先のインデックス座標をもとめる
	calcTargetIndex(targetIndexPos);

	//進先に障害物がある場合移動不可
	if (mMapManager->getMapDataAt(targetIndexPos[0], targetIndexPos[1]) == TileType::WALL ||
		mMapManager->getObjectDataAt(targetIndexPos[0], targetIndexPos[1]) != CharacterType::EMPTY) {
		mTargetPos = mOwner->getPosition();
		return;
	}

	//向きの変更
	//右向き
	if (targetIndexPos[0] - mIndexPos[0] == 1) mOwner->setYRot(XM_PIDIV2);
	//左向き
	else if (targetIndexPos[0] - mIndexPos[0] == -1) mOwner->setYRot(-XM_PIDIV2);
	//上向き
	else if (targetIndexPos[1] - mIndexPos[1] == 1) mOwner->setYRot(0.0f);
	//下向き
	else if (targetIndexPos[1] - mIndexPos[1] == -1) mOwner->setYRot(XM_PI);

	mTargetPos = XMFLOAT3(targetIndexPos[0] * MAPTIPSIZE, 0.0f, targetIndexPos[1] * MAPTIPSIZE); //移動先のワールド座標を計算

	//マップデータや自身のインデックス座標を更新
	mMapManager->setObjectDataAt(mIndexPos[0], mIndexPos[1], CharacterType::EMPTY); //元居た場所を空に
	mMapManager->setObjectDataAt(targetIndexPos[0], targetIndexPos[1], mEnemyType); //移動先のデータを先に更新する
	mIndexPos[0] = targetIndexPos[0]; mIndexPos[1] = targetIndexPos[1]; //インデックス座標の更新
}

void EnemyComponent::attack()
{
	//移動時は実行できない
	if (isMoving) return;
	//エネミーターン時のみ実行
	if (mMapManager->getTurnType() == TurnType::PLAYER) return;
	//行動済みならスキップ
	if (!isActive) return;

	//プレイヤーのインデックス座標を取得
	Player* player = mMapManager->getPlayer();
	int playerIndexPos[2];
	player->getIndexPos(playerIndexPos);

	int diffIndexPos[2] = { playerIndexPos[0] - mIndexPos[0], playerIndexPos[1] - mIndexPos[1] };

	//プレイヤーが左側
	if (diffIndexPos[0] == -1 && diffIndexPos[1] == 0) {
		mOwner->setYRot(-XM_PIDIV2);
	}
	//プレイヤーが右側
	else if (diffIndexPos[0] == 1 && diffIndexPos[1] == 0) {
		mOwner->setYRot(XM_PIDIV2);
	}
	//プレイヤーが下
	else if (diffIndexPos[0] == 0 && diffIndexPos[1] == -1) {
		mOwner->setYRot(XM_PI);
	}
	else if (diffIndexPos[0] == 0 && diffIndexPos[1] == 1) {
		mOwner->setYRot(0.0f);
	}
	else {
		return;
	}

	int damage = max(0, mPower - player->getDefense());
	player->giveDamage(damage);

	finishAct();
}

void EnemyComponent::finishAct()
{
	mMapManager->moveToPlayerTurn();
	isActive = false;

}

void EnemyComponent::calcTargetIndex(int(&targetIndex)[2])
{
	switch (mState) {
	case MovePattern::RANDOM:
		randomWalk(targetIndex);
		break;
	case MovePattern::ASTAR:
		Astar(targetIndex);
		break;
	case MovePattern::SENSE: 
		//索敵範囲内プレイヤーがいた場合
		if (mDistPlayer <= mSenseRange) {
			Astar(targetIndex);
		}
		//索敵範囲内プレイヤーがいない場合
		else {
			randomWalk(targetIndex);
		}
	}

}

void EnemyComponent::Astar(int(&targetIndex)[2])
{
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList; //次に探索するマスのリスト
	std::vector<std::vector<Cell>> grid(mMapManager->getMapSize(), std::vector<Cell>(mMapManager->getMapSize()));

	openList.push({ mIndexPos[0], mIndexPos[1], 0 }); //スタート地点
	grid[mIndexPos[0]][mIndexPos[1]].gCost = 0;
	grid[mIndexPos[0]][mIndexPos[1]].isClosed = true;


	int playerIndex[2];
	mMapManager->getPlayer()->getIndexPos(playerIndex);

	while (!openList.empty()) {
		//openListの先頭ノードをカレントノードに
		Node current = openList.top();
		openList.pop(); //先頭から削除

		//クローズドリストの更新
		grid[current.x][current.y].isClosed = true;

		//カレントノードがプレイヤーと同じ座標なら終了
		if (current.x == playerIndex[0] && current.y == playerIndex[1]) {
			break;
		}

		//周囲4マスのコストを計算
		int hCost = 0;
		int gCost = 0;
		int fCost = 0;
		int nextIndex[2] = { 0, 0 };
		for (int direct = 0; direct < 4; direct++) {
			switch (direct) {
			case 0:
				//上
				nextIndex[0] = current.x; nextIndex[1] = current.y + 1;
				break;
			case 1:
				//下
				nextIndex[0] = current.x; nextIndex[1] = current.y - 1;
				break;
			case 2:
				//右
				nextIndex[0] = current.x + 1; nextIndex[1] = current.y;
				break;
			case 3:
				//左
				nextIndex[0] = current.x - 1; nextIndex[1] = current.y;
				break;
			}

			//探索するマスが移動可能かつ探索していないか判定
			if (nextIndex[0] >= 0 && nextIndex[0] < mMapManager->getMapSize()
				&& nextIndex[1] >= 0 && nextIndex[1] < mMapManager->getMapSize()
				&& !grid[nextIndex[0]][nextIndex[1]].isClosed
				&& mMapManager->getMapDataAt(nextIndex[0], nextIndex[1]) != TileType::WALL
				) {
				gCost = grid[current.x][current.y].gCost + 1;
				//よりGコストの小さいルートを見つけた場合、親ノードとGコストを更新
				if (grid[nextIndex[0]][nextIndex[1]].gCost > gCost) {
					grid[nextIndex[0]][nextIndex[1]].parentX = current.x;
					grid[nextIndex[0]][nextIndex[1]].parentY = current.y;
					grid[nextIndex[0]][nextIndex[1]].gCost = gCost;
					hCost = abs(nextIndex[0] - playerIndex[0]) + abs(nextIndex[1] - playerIndex[1]); //プレイヤーまでのマンハッタン距離
					fCost = hCost + gCost;
					openList.push({ nextIndex[0] , nextIndex[1], fCost });
				}
			}
		}
	}

	if (grid[playerIndex[0]][playerIndex[1]].isClosed == false) return; //プレイヤーにたどり着けない場合何も返さない
	int childIndex[2];
	childIndex[0] = playerIndex[0];
	childIndex[1] = playerIndex[1];

	//次に進むマスを親ノードから探索
	while (true) {
		int parentIndex[2] = { grid[childIndex[0]][childIndex[1]].parentX, grid[childIndex[0]][childIndex[1]].parentY };
		if (parentIndex[0] == mIndexPos[0] && parentIndex[1] == mIndexPos[1]) {
			targetIndex[0] = childIndex[0];
			targetIndex[1] = childIndex[1];
			return;
		}

		childIndex[0] = parentIndex[0]; childIndex[1] = parentIndex[1];


	}


}

void EnemyComponent::randomWalk(int(&targetIndex)[2])
{
	targetIndex[0] = mIndexPos[0]; targetIndex[1] = mIndexPos[1];

	int direction = 1 << Random::dist(0,4);
	switch (direction) {
		case Direction::UP:
			targetIndex[1] += 1;
			break;
		case Direction::DOWN:
			targetIndex[1] -= 1;
			break;
		case Direction::RIGHT:
			targetIndex[0] += 1;
			break;
		case Direction::LEFT:
			targetIndex[0] -= 1;
			break;
		case 1 << 4:
			break;
	}

	return;

}
