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
#include "Treasure.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "PlayerAttackComponent.h"
#include "PlayerMoveComponent.h"

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

	//移動用コンポーネントの生成
	auto moveComponent = std::make_unique<PlayerMoveComponent>(scene, *this);
	mMoveComponent = moveComponent.get();
	addComponent(std::move(moveComponent));

	//攻撃用コンポーネントの生成
	auto singleAttack = std::make_unique<PlayerSingleAttackComponent>(scene, *this);
	auto doubleAttack = std::make_unique<PlayerDoubleAttackComponent>(scene, *this);
	mAttackComponents[AttackType::SINGLE] = singleAttack.get();	//攻撃タイプに対応する攻撃コンポーネントをマップに格納
	mAttackComponents[AttackType::DOUBLE] = doubleAttack.get();
	addComponent(std::move(singleAttack));
	addComponent(std::move(doubleAttack));
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

std::vector<int> Player::getIndexPos()
{
	std::vector<int> pos = { mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1] };
	return pos;
}

int Player::getCurrentHP()
{
	return mCharacter->getHP();
}

int Player::getCurrentAP()
{
	return mAP;
}

int Player::getPower()
{
	return mCharacter->getPower();
}

int Player::getDefence()
{
	return mCharacter->getDefence();
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

void Player::setIndexPos(int x, int y)
{
	mCharacter->setIndexPos(x, y);
}

//プレイヤーにダメージを与える
void Player::giveDamage(int damage)
{
	//直接HPを減らさない
	//行動中に受けたダメージを全て加算し、行動終了後にまとめてHPから引くようにする
	mPendingDamage += damage;
}

void Player::startAct()
{
	isActing = true;
}

void Player::moveToEnemyTurn()
{
	mScene.moveToEnemyTurn();
}

void Player::attack()
{
	//敵ターン時は実行不可
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0)return;

	//攻撃タイプに対応する攻撃コンポーネントをマップから取得
	auto attackComponentIter = mAttackComponents.find(mPlayerData.attackType);
	if (attackComponentIter == mAttackComponents.end()) return;	//攻撃タイプに対応する攻撃コンポーネントが存在しない場合は処理を終了

	attackComponentIter->second->execute();	//攻撃コンポーネントの攻撃処理を実行
}

void Player::move(Direction direction)
{
	//プレイヤーターン時のみ実行
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//行動回数が0の場合実行不可
	if (mAP == 0) return;

	mMoveComponent->move(direction);	//移動用コンポーネントの移動処理を実行
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
	endAct();
	moveToEnemyTurn();
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
	endAct();
	moveToEnemyTurn();
}

void Player::getTreasure()
{
	//プレイヤーターン時のみ実行
	if (mScene.getTurnType() == TurnType::ENEMY) return;
	//移動、回転中は実行不可
	if (isActing || isRotating) return;
	//残り行動回数が0の場合実行不可
	if (mAP == 0) return;

	//前方に宝箱があるか判定
	std::vector<int> searchPos = {mCharacter->getIndexPos()[0], mCharacter->getIndexPos()[1]};

	switch (mCharacter->getDirection()) {
	case Direction::UP:
		searchPos[1] += 1;
		break;
	case Direction::DOWN:
		searchPos[1] -= 1;
		break;
	case Direction::RIGHT:
		searchPos[0] += 1;
		break;
	case Direction::LEFT:
		searchPos[0] -= 1;
		break;
	}

	const int tileData = mScene.getTileDataAt(searchPos[0], searchPos[1]);

	//前方に宝箱がない場合は何もしない
	if (tileData != TileType::TREASURE) return;

	//宝箱のポインタを取得
	Treasure* treasure = mScene.getTreasureAt(searchPos[0], searchPos[1]);
	//宝箱が設定されていなかったら何もしない
	if (treasure == nullptr) return;

	//宝箱からアイテムを取得
	treasure->open();

	//取得処理


	//終了処理
	endAct();
	moveToEnemyTurn();
}

void Player::endAct()
{
	isActing = false;

	//残り行動回数を減らす
	mAP--;
	//UIの更新
	if(mAP >= 0) mScene.updateAPUI();
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

	endAct();
	moveToEnemyTurn();
	mScene.getTurnObserver().stop();	//ターンオブザーバーを止める(敵の行動も止まる)
}

TreasureWindow::TreasureWindow(Scene& scene, const std::string& itemID)
	: Object(scene, "TREASURE_WINDOW")
{
}

void TreasureWindow::inputActor()
{
	
}
