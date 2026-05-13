#include "Player.h"
#include "TurnObserver.h"
#include "CameraComponent.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"
#include "json.hpp"
#include "Math.h"
#include "timer.h"
#include "CharacterComponent.h"
#include "DamageText.h"
#include "EnemyComponent.h"
#include <windows.h>
#include <algorithm>
#include "Game.h"
#include "Graphic.h"
#include "ItemManager.h"
#include "input.h"
#include "PlayerManager.h"
#include "AudioManager.h"
#include "MiniMap.h"
#include "DungeonScene.h"
#include "Resource.h"
#include "EndWindow.h"

Player::Player(DungeonScene& scene, float x, float y)
	: Actor(scene),
	mScene(scene),
	mPlayerManager(scene.getGame().getPlayerManager()),
	mItemManager(scene.getGame().getItemManager()),
	mPlayerData(mPlayerManager.getPlayerData())
{
	//初期化
	mPosition		 = { x, 0.8f, y };
	mTargetPos	     = mPosition;
	mTargetRot		 = mRotation;
	isActing		 = false;
	isRotating		 = false;
	mFlashTimer		 = 0.0f;
	mSelectItemIndex = 0;

	//カメラコンポーネントの生成：プレイヤーの視点を制御
	std::unique_ptr camera = std::make_unique<CameraComponent>(*this);
	camera->setActive(true);
	mCamera = camera.get();
	addComponent(std::move(camera));

	//スポットライトコンポーネントの生成：プレイヤーの視点方向を照らすライト
	std::unique_ptr<SpotLightComponent> spotLight = std::make_unique<SpotLightComponent>(*this);
	spotLight->setActive(true);
	spotLight->setColor(XMFLOAT4(1.0f, 0.9f, 0.8f, 1.0f));
	spotLight->setIntensity(20.0f);
	spotLight->setRange(50.0f);
	spotLight->setUAngle(XMConvertToRadians(10.0f));
	spotLight->setPAngle(XMConvertToRadians(40.0f));
	spotLight->setOffsetPos(XMFLOAT4(0.0f, 0.2f, 0.0f, 0.0f));
	addComponent(std::move(spotLight));

	//プレイヤーデータから各種パラメータを取得
	//探索道具の効果をステータスに適用する
	mPlayerManager.applyToolParamater();

	mMoveSpeed		= mPlayerData.moveSpeed;		//移動速度
	mRotSpeed		= mPlayerData.rotSpeed;			//回転速度
	mFlashDuration  = mPlayerData.flushDuration;	//ダメージを受けたときの点滅時間
	mAP = mPlayerData.actionLimit;					//APの初期値

	//キャラクターコンポーネントの生成
	auto character = std::make_unique<CharacterComponent>(*this, scene);
	character->setDirection(Direction::UP);
	character->setIndexPos(static_cast<int>(std::round(x / MAPTIPSIZE)), static_cast<int>(std::round(y / MAPTIPSIZE)));
	character->setMaxHP(mPlayerData.maxHp);
	character->setHP(mPlayerData.hp);
	character->setPower(mPlayerData.power);	
	character->setDefense(mPlayerData.defence);
	mCharacter = character.get();
	addComponent(std::move(character));

}

void Player::inputActor()
{
	//敵ターン、HP0以下のときは入力を受け付けない
	if (mScene.getTurnType() == TurnType::END || mCharacter->getHP() <= 0) return;

	//---移動---
	if (isKeyPressed('A')) {
		move(Direction::LEFT);
	}
	if (isKeyPressed('D')) {
		move(Direction::RIGHT);
	}
	if (isKeyPressed('W')) {
		move(Direction::UP);
	}
	if (isKeyPressed('S')) {
		move(Direction::DOWN);
	}
	//---視点移動---
	if (isKeyPressed(VK_RIGHT) || isKeyPressed('L')) {
		rotate(Direction::RIGHT);
	}
	if (isKeyPressed(VK_LEFT) || isKeyPressed('J')) {
		rotate(Direction::LEFT);
	}
	//---アクション---
	if (isKeyJustPressed(VK_RETURN) || isKeyJustPressed('K')) {
		attack();			//目の前の敵を攻撃
		collect();			//リソースを回収
		moveNextFloor();	//次の階へ移動
	}
	//---アイテム使用---
	if (isKeyJustPressed('I')) {
		useItem();
	}
	if (isKeyJustPressed('U')) {
		selectPreviousItem();
	}
	if (isKeyJustPressed('O')) {
		selectNextItem();
	}
	
}

//毎フレームの更新処理
void Player::updateActor()
{
	//移動処理
	if (isActing) {
		XMFLOAT3 diffPos = mTargetPos - mPosition;
		float moveLength = deltaTime * mMoveSpeed;

		//位置の更新  diffPos(現在地と目的地の差分)よりmoveLength(移動距離)が大きくなるまで現在地にmoveLengthを加算
		if (fabsf(diffPos.x) > moveLength || fabsf(diffPos.y) > moveLength || fabsf(diffPos.z) > moveLength) {
			mPosition = mPosition + Math::normalize(diffPos) * moveLength;
		}
		else {
			//目的地にピッタリ合わせる
			mPosition = mTargetPos;
			isActing = false;
		}
	}

	//カメラ回転時の処理
	if (isRotating) {
		XMFLOAT3 diffRot = mTargetRot - mRotation;
		float rotLength = deltaTime * mRotSpeed;

		////回転の更新 diffRot(現在の回転角と目的の回転角の差分)よりrotLength(回転量)が大きくなるまで現在の回転にrotLengthを加算
		if (fabsf(diffRot.x) > rotLength || fabsf(diffRot.y) > rotLength || fabsf(diffRot.z) > rotLength) {

			mRotation = mRotation + Math::normalize(diffRot) * rotLength;
		}
		//終了時の処理
		else {
			//目的の回転角にピッタリ合わせる
			mRotation = mTargetRot;
			isRotating = false;
		}
	}

	//ダメージを受けたときの処理
	if (!isActing) damagedProcess();

	//ダメージを受けたときの点滅処理
	updateFlash();
}

//アクター破棄時の処理
void Player::endProcessActor()
{
	//ダンジョンシーン中のデータをプレイヤーマネージャーに保存する
	PlayerManager& player = mScene.getGame().getPlayerManager();
	player.setHP(mCharacter->getHP());
	//点滅強度をリセットする
	mScene.getGame().getGraphic().updateDamageFlashIntensity(0.0f);
}

//---getter---
int Player::getDirection()
{
	return mCharacter->getDirection();
}

void Player::getIndexPos(int(&pos)[2])
{
	pos[0] = mCharacter->getIndexPos()[0];
	pos[1] = mCharacter->getIndexPos()[1];
}

int Player::getCurrentHP()
{
	return mCharacter->getHP();
}

int Player::getCurrentAP()
{
	return mAP;
}

int Player::getSelectItemIndex()
{
	return mSelectItemIndex;
}

bool Player::getIsActing()
{
	return isActing;
}

const std::string& Player::getSelectItemID()
{
	return mPlayerManager.getInventoryItem(mSelectItemIndex);
}

//プレイヤーにダメージを与える
void Player::giveDamage(int damage)
{
	//直接HPを減らさない
	//行動中に受けたダメージを全て加算し、行動終了後にまとめてHPから引くようにする
	mPendingDamage += damage;
}

void Player::attack()
{
	//敵ターン時は実行不可
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0)return;

	//前方のエネミーのポインターを取得
	EnemyComponent* target = nullptr;
	switch (mCharacter->getDirection()) {
	case Direction::UP:
		target = mScene.getEnemyFromIndexPos(mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1] + 1);
		break;
	case Direction::DOWN:
		target = mScene.getEnemyFromIndexPos(mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1] - 1);
		break;
	case Direction::RIGHT:
		target = mScene.getEnemyFromIndexPos(mCharacter->getIndexPos()[0] + 1, mCharacter->getIndexPos()[1]);
		break;
	case Direction::LEFT:
		target = mScene.getEnemyFromIndexPos(mCharacter->getIndexPos()[0] - 1, mCharacter->getIndexPos()[1]);
		break;
	}

	//前方に敵がいない場合は攻撃できない
	if (target == nullptr) { 
		return; 
	}

	//ダメージの計算
	int damage = max(mCharacter->getPower() - target->getDefense(), 0);
	target->giveDamage(damage); //ダメージを与える

	//ダメージエフェクト
	target->startFlash();	//敵を点滅させる
	calcDamageText(target->getPosition(), damage);	//ダメージ値を描画
	mScene.getGame().getAudioManager().playSE("DAMAGE1");

	//ターン経過
	turnEnd();
}

void Player::calcDamageText(const XMFLOAT3& targetPos, int val)
{
	//ダメージ値を生成する位置を計算
	XMFLOAT3 textPos = targetPos;
	textPos.y += 0.8f; //少しだけ上に

	//ダメージ値はビルボード処理でカメラの方向を向くようにするため、プレイヤーの前方ベクトルと右ベクトルを計算する
	XMFLOAT3 front = Math::rotateY(XMFLOAT3(0.0f, 0.0f, 1.0f), mRotation.y); //プレイヤーの前方ベクトル
	XMFLOAT3 right = Math::rotateY(front, -XM_PIDIV2);	//プレイヤーの右ベクトル
	textPos = textPos - front;	//敵オブジェクトより手前に

	int digit = 0;			//桁数
	int value = val;		//表示したい数値
	std::vector<int> num(DamageTextGenerator::MaxNum);	//各桁の値, 最大数で配列を初期化しておく

	//桁数と各桁の値を取得
	if (value == 0) {
		num[0] = val;
		digit = 1;
	}
	while (value > 0) {
		num[digit] = value % 10;
		digit++;
		value = value / 10;
	}

	float DTHalfSize = mScene.getDamageTextNum() * 0.5f;
	//数値が画面中心に来るよう調整
	textPos = textPos + (right * DTHalfSize * 0.5f * (digit - 1));
	//桁ごとの表示位置の調整
	for (int i = 0; i < digit; i++) {
		mScene.createDamageText(textPos, num[i]);
		textPos = textPos - right * DTHalfSize;	//次の桁はカメラの左方向にずらす
	}
}

void Player::move(Direction direction)
{
	//プレイヤーターン時のみ実行
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//行動回数が0の場合実行不可
	if (mAP == 0) return;

	int targetIndexPos[2] = {mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1]};
	//現在の向きと進行方向から移動先のインデックス座標を計算
	calcMoveDirectionToIndexPos(direction, targetIndexPos);

	//進先に障害物がある場合移動不可
	if (mScene.getTileDataAt(targetIndexPos[0], targetIndexPos[1]) == TileType::WALL ||
		mScene.getCharacterDataAt(targetIndexPos[0], targetIndexPos[1]) != CharacterType::EMPTY) return;

	//移動前の座標を空に
	mScene.setCharacterDataAt(mCharacter->getIndexPosInt(), CharacterType::EMPTY);
	//マップ上のオブジェクトデータ更新
	mScene.setCharacterDataAt(targetIndexPos[0], targetIndexPos[1], CharacterType::PLAYER);
	//プレイヤーのインデックス座標の更新
	mCharacter->setIndexPos(targetIndexPos[0], targetIndexPos[1]);

	mTargetPos = XMFLOAT3(static_cast<float>(targetIndexPos[0]) * MAPTIPSIZE, mPosition.y, static_cast<float>(targetIndexPos[1]) * MAPTIPSIZE);
	
	mScene.getGame().getAudioManager().playSE("DUNGEON_FOOTSTEP");
	turnEnd();

}	

void Player::rotate(Direction direction)
{
	//プレイヤーターン時のみ実行
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;

	switch (direction) {
	case Direction::RIGHT:
		mTargetRot = mRotation + XMFLOAT3(0, XM_PIDIV2, 0);
		mCharacter->turnRight(); //向きの更新

		break;
	case Direction::LEFT:
		mTargetRot = mRotation - XMFLOAT3(0, XM_PIDIV2, 0);
		mCharacter->turnLeft(); //向きの更新
		break;
	}

	isRotating = true;
	mScene.updateMiniMapDirection();	//ミニマップのアイコンの向きを更新
}

void Player::calcMoveDirectionToIndexPos(Direction moveDirection, int (&indexPos)[2])
{
	//動く方向と向いている方角の論理和から、最終的に動く向きをもとめる
	//moveDirection(移動方向)					　: 0001(後),0010(右),0100(上),1000(左)
	//mCharacter->getDirection() (向いている方角) : 0001(南),0010(東),0100(北),1000(西)
	int index = moveDirection | mCharacter->getDirection(); 

	//indexは以下の10通りになる
	//東　0100, 0001, 1010
	//西　0101, 0010, 1000
	//北　0110, 1001
	//南　1100, 0011

	//indexから移動先のインデックス座標を計算
	switch (index) {
	//東に移動
	case 0b0100:
	case 0b0001:
	case 0b1010:
		indexPos[1] += 1;
		break;
	//西に移動
	case 0b0101:
	case 0b0010:
	case 0b1000:
		indexPos[1] -= 1;
		break;
	//北に移動
	case 0b0110:
	case 0b1001:
		indexPos[0] += 1;
		break;
	//南に移動
	case 0b1100:
	case 0b0011:
		indexPos[0] -= 1;
		break;
	}

}

void Player::collect()
{
	//プレイヤーターン時のみ実行
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0) return;

	//プレイヤーの位置に資源があるか判定
	auto resource = mScene.getResource(mCharacter->getIndexPosInt());
	//ある場合、Resourceクラスのポインタからリソースを回収する関数を実行
	if (resource) resource->collect();
	//ない場合は何もしない
	else return;

	//SE
	mScene.getGame().getAudioManager().playSE("PICKAXE");
	//メッセージを追加 resourceをx取得した！(現在の総数)
	std::string message = resource->getResourceName() + "を" + std::to_string(resource->getAmount()) + "取得した! (総数"
						 + std::to_string(mItemManager.getResourceNum(resource->getResourceID())) + ")\n";
	mScene.pushMessage(message);

	//ターン経過
	turnEnd();
}

void Player::damagedProcess()
{
	//ダメージの反映 
	if (mPendingDamage <= 0) return;
	int hp = mCharacter->getHP() - mPendingDamage;
	mCharacter->setHP(hp);
	mPendingDamage = 0;

	//UIの更新
	mScene.updateHPUI();

	//死亡処理
	if (hp <= 0) {
		//死亡ウィンドウを表示
		auto endWindow = std::make_unique<EndWindow>(mScene, WindowType::DEAD);
		mScene.addActor(std::move(endWindow));
		return;
	}

	//被ダメージ時の演出
	mFlashTimer = mFlashDuration;	//画面を点滅
	mCamera->startShake();			//カメラを揺らす
	mScene.getGame().getAudioManager().playSE("DAMAGE2");

	
}

void Player::updateFlash()
{
	//ダメージの点滅処理
	if (mFlashTimer > 0.0f) {
		mFlashTimer -= deltaTime;
		float intensity = max(0.0f, mFlashTimer / mFlashDuration);
		mScene.getGame().getGraphic().updateDamageFlashIntensity(intensity);	//コンスタントバッファに点滅の強度を送る
	}
}

void Player::useItem()
{
	//敵ターン時は実行不可
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0) return;

	//選択中のIndexからアイテムのIDを取得
	const auto& itemID = mPlayerManager.getInventoryItem(mSelectItemIndex);
	if (itemID == "") {
		//アイテムがない場合は何もしない
		return;
	}
	
	//アイテムのIDからアイテムデータを取得
	const ItemData& itemData = mItemManager.getItemData(itemID);

	//アイテムのカテゴリーから効果を発揮
	//HP回復
	if (itemData.category == "HP_RECOVER") {
		mCharacter->addHP(itemData.value);
		mScene.updateHPUI();
		mScene.getGame().getAudioManager().playSE("RECOVER1");
	}
	//AP回復
	else if (itemData.category == "AP_RECOVER") {
		mAP += itemData.value;
		if (mAP > mPlayerData.actionLimit) mAP = mPlayerData.actionLimit + 1;
		mScene.getGame().getAudioManager().playSE("RECOVER1");
	}

	//使用したアイテムをインベントリーから削除
	mPlayerManager.removeInventory(mSelectItemIndex);

	//UIの更新
	mScene.updateItemUI();
	mScene.updateItemFrame();

	//ターン経過
	turnEnd();
}

void Player::turnEnd()
{
	//ターンをエネミーターンに変更
	mScene.moveToEnemyTurn();
	//残り行動回数を減らす
	mAP--;
	//UIの更新
	if(mAP >= 0) mScene.updateAPUI();
	//行動中フラグをtureにする
	isActing = true;
}

void Player::selectNextItem()
{
	//アイテムカーソルを右に移動
	if (mSelectItemIndex >= mPlayerData.storageSize - 1) return;
	mSelectItemIndex++;
	mScene.updateItemFrame();
	mScene.getGame().getAudioManager().playSE("UI_MOVE1");
}

void Player::selectPreviousItem()
{
	//アイテムカーソルを左に移動
	if (mSelectItemIndex <= 0) return;
	mSelectItemIndex--;
	mScene.updateItemFrame();
	mScene.getGame().getAudioManager().playSE("UI_MOVE1");
}

void Player::moveNextFloor()
{
	//敵ターン時は実行不可
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0)return;

	//目の前に階段があるか判定
	bool isGoal = false;
	switch (mCharacter->getDirection()) {
	case Direction::UP:
		isGoal = mScene.getTileDataAt(mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1] + 1) == TileType::GOAL;
		break;
	case Direction::DOWN:
		isGoal = mScene.getTileDataAt(mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1] - 1) == TileType::GOAL;
		break;
	case Direction::RIGHT:
		isGoal = mScene.getTileDataAt(mCharacter->getIndexPos()[0] + 1, mCharacter->getIndexPos()[1]) == TileType::GOAL;
		break;
	case Direction::LEFT:
		isGoal = mScene.getTileDataAt(mCharacter->getIndexPos()[0] - 1, mCharacter->getIndexPos()[1]) == TileType::GOAL;
		break;
	}

	if (!isGoal) return;

	//階段があった場合の処理
	//現在はクリアウィンドウを表示するだけだが、次の階のマップを生成して移動する処理などもここに書くことになる
	auto goalWindow = std::make_unique<EndWindow>(mScene, WindowType::GOAL);
	mScene.addActor(std::move(goalWindow));

	turnEnd();
	mScene.getTurnObserver().stop();	//ターンオブザーバーを止める(敵の行動も止まる)
}
