#include "Player.h"
#include "CameraComponent.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"
#include "Math.h"
#include "timer.h"
#include "CharacterComponent.h"
#include "DamageText.h"
#include "EnemyComponent.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
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
	mItemManager(scene.getGame().getItemManager())
{
	mPosition = { x, 0.8f, y };
	mTargetPos = mPosition;
	mTargetRot = mRotation;
	isActing = false;
	isRotating = false;
	mFlashTimer = 0.0f;
	mSelectItemIndex = 0;

	//プレイヤーデータの取得
	mPlayerManager.applyToolParamater();
	const PlayerData& data = mPlayerManager.getPlayerData();

	//移動速度、回転速度、点滅時間の設定
	mMoveSpeed = data.moveSpeed;
	mRotSpeed = data.rotSpeed;
	mFlashDuration = data.flushDuration;

	//カメラの生成
	std::unique_ptr camera = std::make_unique<CameraComponent>(*this);
	camera->setActive(true);
	mCamera = camera.get();
	addComponent(std::move(camera));

	//スポットライトの生成
	std::unique_ptr<SpotLightComponent> spotLight = std::make_unique<SpotLightComponent>(*this);
	spotLight->setActive(true);
	spotLight->setColor(XMFLOAT4(1.0f, 0.9f, 0.8f, 1.0f));
	spotLight->setIntensity(20.0f);
	spotLight->setRange(50.0f);
	spotLight->setUAngle(XMConvertToRadians(10.0f));
	spotLight->setPAngle(XMConvertToRadians(40.0f));
	addComponent(std::move(spotLight));

	//キャラクターコンポーネントの生成
	auto character = std::make_unique<CharacterComponent>(*this, scene);
	character->setDirection(Direction::UP);
	character->setIndexPos(static_cast<int>(std::round(x / MAPTIPSIZE)), static_cast<int>(std::round(y / MAPTIPSIZE)));
	character->setMaxHP(data.maxHp);
	character->setHP(data.hp);
	character->setPower(data.power);	
	character->setDefense(data.defence);
	mCharacter = character.get();
	addComponent(std::move(character));

	//行動回数制限の取得
	mMaxAP = data.actionLimit;
	mAP = mMaxAP;

	//最大所持アイテム数
	mStorageSize = data.storageSize;

}

void Player::inputActor()
{
	if (mScene.getTurnType() == TurnType::END || mCharacter->getHP() <= 0) return;

	if (GetAsyncKeyState('A')) {
		move(Direction::LEFT);
	}
	if (GetAsyncKeyState('D')) {
		move(Direction::RIGHT);
	}
	if (GetAsyncKeyState('W')) {
		move(Direction::UP);
	}
	if (GetAsyncKeyState('S')) {
		move(Direction::DOWN);
	}
	if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('L')) {
		rotate(Direction::RIGHT);
	}
	if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('J')) {
		rotate(Direction::LEFT);
	}
	if (isKeyJustPressed(VK_RETURN) || isKeyJustPressed('K')) {
		attack();
	    collect();
	}
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

void Player::updateActor()
{
	//移動処理
	if (isActing) {
		//移動処理
		XMFLOAT3 diffPos = mTargetPos - mPosition;

		float moveLength = deltaTime * mMoveSpeed;

		//位置の更新
		if (fabsf(diffPos.x) > moveLength || fabsf(diffPos.y) > moveLength || fabsf(diffPos.z) > moveLength) {
			mPosition = mPosition + Math::normalize(diffPos) * moveLength;
		}
		else {
			//移動終了時の処理
			mPosition = mTargetPos;
			
			isActing = false;
		}
	}
	//カメラ回転時の処理
	if (isRotating) {
		XMFLOAT3 diffRot = mTargetRot - mRotation;
		float rotLength = deltaTime * mRotSpeed;

		////回転の更新
		if (fabsf(diffRot.x) > rotLength || fabsf(diffRot.y) > rotLength || fabsf(diffRot.z) > rotLength) {

			mRotation = mRotation + Math::normalize(diffRot) * rotLength;
		}
		//終了時の処理
		else {
			mRotation = mTargetRot;
			isRotating = false;
		}
	}

	if (!isActing) damagedProcess();
	updateFlash();
}

void Player::endProcessActor()
{
	PlayerManager& player = mScene.getGame().getPlayerManager();
	player.setHP(mCharacter->getHP());
	mScene.getGame().getGraphic().updateDamageFlashIntensity(0.0f);
}

int Player::getDirection()
{
	return mCharacter->getDirection();
}

void Player::getIndexPos(int(&pos)[2])
{
	pos[0] = mCharacter->getIndexPos()[0];
	pos[1] = mCharacter->getIndexPos()[1];
}

int Player::getMaxHP()
{
	return mCharacter->getMaxHP();
}

int Player::getHP()
{
	return mCharacter->getHP();
}

int Player::getPower()
{
	return mCharacter->getPower();
}

int Player::getDefense()
{
	return mCharacter->getDefense();
}

int Player::getAP()
{
	return mAP;
}

int Player::getMaxAP()
{
	return mMaxAP;
}

int Player::getStorageSize()
{
	return mStorageSize;
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

void Player::giveDamage(int damage)
{
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

	//前方のエネミーを取得
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

	if (target == nullptr) { 
		return; 
	}

	//ダメージの計算
	int damage = max(mCharacter->getPower() - target->getDefense(), 0);
	target->giveDamage(damage); //ダメージを与える

	//ダメージエフェクト
	target->startFlash(); //敵を点滅させる
	calcDamageText(target->getPosition(), damage);
	mScene.getGame().getAudioManager().playSE("DAMAGE1");

	//ターン経過
	turnEnd();
}

void Player::calcDamageText(const XMFLOAT3& targetPos, int val)
{
	XMFLOAT3 textPos = targetPos;
	textPos.y += 0.8f; //少しだけ上に

	XMFLOAT3 front = Math::rotateY(XMFLOAT3(0.0f, 0.0f, 1.0f), mRotation.y);//前方ベクトル
	XMFLOAT3 right = Math::rotateY(front, -XM_PIDIV2);//右ベクトル
	textPos = textPos - front;	//敵オブジェクトより手前に

	int digit = 0;			//桁数
	int value = val;		//表示したい数値
	std::vector<int> num;	//各桁の値

	//桁数と各桁の値を取得
	if (value == 0) {
		num.push_back(value);
		digit = 1;
	}
	while (value > 0) {
		digit++;
		num.push_back(value % 10);
		value = value / 10;
	}

	float DTHalfSize = mScene.getDamageTextNum() * 0.5f;
	//数値が画面中心に来るよう調整
	textPos = textPos + (right * DTHalfSize * 0.5f * (digit - 1));
	//桁ごとの表示位置の調整
	for (int i = 0; i < digit; i++) {
		mScene.createDamageText(textPos, num[i]);
		textPos = textPos - right * DTHalfSize;
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
	//進行する差分のインデックスを取得
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
	mScene.updateMiniMapDirection();
}

void Player::calcMoveDirectionToIndexPos(Direction moveDirection, int (&indexPos)[2])
{
	//動く方向と向いている方向から、最終的に動く向きをもとめる
	int index = moveDirection | mCharacter->getDirection(); 

	//インデックス座標の変位をもとめる
	switch (index) {
	case 0b0100:
	case 0b0001:
	case 0b1010:
		indexPos[1] += 1;
		break;
	case 0b0101:
	case 0b0010:
	case 0b1000:
		indexPos[1] -= 1;
		break;
	case 0b0110:
	case 0b1001:
		indexPos[0] += 1;
		break;
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


	auto resource = mScene.getResource(mCharacter->getIndexPosInt());
	if (resource) resource->collect();
	else return;

	//SE
	mScene.getGame().getAudioManager().playSE("PICKAXE");
	//メッセージを追加 resourceをx取得した！(現在の総数)
	std::string message = resource->getResourceName() + "を" + std::to_string(resource->getAmount()) + "取得した!("
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
		auto endWindow = std::make_unique<EndWindow>(mScene);
		mScene.addActor(std::move(endWindow));
		return;
	}

	//被ダメージ時の演出
	mFlashTimer = mFlashDuration;
	mCamera->startShake();
	mScene.getGame().getAudioManager().playSE("DAMAGE2");

	
}

void Player::updateFlash()
{
	//ダメージの点滅処理
	if (mFlashTimer > 0.0f) {
		mFlashTimer -= deltaTime;
		float intensity = max(0.0f, mFlashTimer / mFlashDuration);
		mScene.getGame().getGraphic().updateDamageFlashIntensity(intensity);
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

	//アイテムのIDを取得
	const auto& itemID = mPlayerManager.getInventoryItem(mSelectItemIndex);
	if (itemID == "") {
		return;
	}
	
	//アイテムデータを取得
	const ItemData& itemData = mItemManager.getItemData(itemID);

	//アイテムのカテゴリーから効果を発揮
	if (itemData.category == "HP_RECOVER") {
		mCharacter->addHP(itemData.value);
		mScene.updateHPUI();
		mScene.getGame().getAudioManager().playSE("RECOVER1");
	}
	else if (itemData.category == "AP_RECOVER") {
		mAP += itemData.value;
		if (mAP > mMaxAP) mAP = mMaxAP + 1;
		mScene.getGame().getAudioManager().playSE("RECOVER1");
	}

	//インベントリーから削除
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
	if (mSelectItemIndex >= mStorageSize - 1) return;
	mSelectItemIndex++;
	mScene.updateItemFrame();
	mScene.getGame().getAudioManager().playSE("UI_MOVE1");
}

void Player::selectPreviousItem()
{
	if (mSelectItemIndex <= 0) return;
	mSelectItemIndex--;
	mScene.updateItemFrame();
	mScene.getGame().getAudioManager().playSE("UI_MOVE1");
}
