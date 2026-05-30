#include "PlayerMoveComponent.h"
#include "Player.h"
#include "DungeonScene.h"
#include "Game.h"
#include "AudioManager.h"
#include "Math.h"

const float PlayerMoveComponent::MoveSpeed = 5.0f;
PlayerMoveComponent::PlayerMoveComponent(DungeonScene& scene, Player& player)
	: Component(player), mScene(scene), mPlayer(player)
{
	isMoving = false;
	mTargetPos = player.getPosition();
}

void PlayerMoveComponent::updateComponent()
{
	if (isMoving) {
		const XMFLOAT3 currentPos = mPlayer.getPosition();
		const XMFLOAT3 diffPos = mTargetPos - currentPos;
		float moveLength = deltaTime * MoveSpeed;

		//位置の更新  diffPos(現在地と目的地の差分)よりmoveLength(移動距離)が大きくなるまで現在地にmoveLengthを加算
		if (fabsf(diffPos.x) > moveLength || fabsf(diffPos.y) > moveLength || fabsf(diffPos.z) > moveLength) {
			const XMFLOAT3 nextPos = currentPos + Math::normalize(diffPos) * moveLength;
			mPlayer.setPosition(nextPos);
			return;
		}

		//目的地に接近したらピッタリ合わせる
		mPlayer.setPosition(mTargetPos);
		mPlayer.endAct();	//行動終了	
		isMoving = false;
	}
}

void PlayerMoveComponent::move(Direction direction)
{
	//現在の向きと進行方向から移動先のインデックス座標を計算
	const std::vector<int> targetIndexPos = calcTargetIndexPos(direction);

	//進先に障害物がある場合移動不可
	const int tileData = mScene.getTileDataAt(targetIndexPos[0], targetIndexPos[1]);
	const int characterData = mScene.getCharacterDataAt(targetIndexPos[0], targetIndexPos[1]);
	if (tileData == TileType::WALL ||
		tileData == TileType::TREASURE ||
		characterData != CharacterType::EMPTY) return;

	//移動前の座標を空に
	const std::vector<int> currentIndexPos = mPlayer.getIndexPos();
	mScene.setCharacterDataAt(currentIndexPos[0], currentIndexPos[1], CharacterType::EMPTY);
	//マップ上のオブジェクトデータ更新
	mScene.setCharacterDataAt(targetIndexPos[0], targetIndexPos[1], CharacterType::PLAYER);
	//プレイヤーのインデックス座標の更新
	mPlayer.setIndexPos(targetIndexPos[0], targetIndexPos[1]);

	mTargetPos = XMFLOAT3(static_cast<float>(targetIndexPos[0]) * MAPTIPSIZE, mPlayer.getPosition().y, static_cast<float>(targetIndexPos[1]) * MAPTIPSIZE);
	
	mScene.getGame().getAudioManager().playSE("DUNGEON_FOOTSTEP");

	mPlayer.startAct();		   //行動開始
	mPlayer.moveToEnemyTurn(); //敵ターンに移行
	isMoving = true;
}

std::vector<int> PlayerMoveComponent::calcTargetIndexPos(Direction moveDirection)
{
	//動く方向と向いている方角の論理和から、最終的に動く向きをもとめる
	//moveDirection(移動方向)					　: 0001(後),0010(右),0100(上),1000(左)
	//mCharacter->getDirection() (向いている方角) : 0001(南),0010(東),0100(北),1000(西)
	int index = moveDirection | mPlayer.getDirection(); 

	//indexは以下の10通りになる
	//東　0100, 0001, 1010
	//西　0101, 0010, 1000
	//北　0110, 1001
	//南　1100, 0011

	std::vector<int> targetIndexPos = mPlayer.getIndexPos();
	//indexから移動先のインデックス座標を計算
	switch (index) {
	//東に移動
	case 0b0100:
	case 0b0001:
	case 0b1010:
		targetIndexPos[1] += 1;
		break;
	//西に移動
	case 0b0101:
	case 0b0010:
	case 0b1000:
		targetIndexPos[1] -= 1;
		break;
	//北に移動
	case 0b0110:
	case 0b1001:
		targetIndexPos[0] += 1;
		break;
	//南に移動
	case 0b1100:
	case 0b0011:
		targetIndexPos[0] -= 1;
		break;
	}

	return targetIndexPos;

}
