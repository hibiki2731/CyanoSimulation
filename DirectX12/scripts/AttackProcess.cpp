#include "AttackProcess.h"
#include "Player.h"
#include "DungeonScene.h"
#include "EnemyComponent.h"
#include "Math.h"
#include "DamageText.h"
#include "Game.h"
#include "AudioManager.h"

IAttackProcess::IAttackProcess(DungeonScene& scene, Player& player)
	:Actor(scene), mScene(scene), mPlayer(player)
{
}

EnemyComponent* IAttackProcess::searchTargetEnemy()
{
	//前方のエネミーのポインターを取得
	EnemyComponent* target = nullptr;
	std::vector<int> playerPos = mPlayer.getIndexPos();
	switch (mPlayer.getDirection()) {
	case Direction::UP:
		target = mScene.getEnemyFromIndexPos(playerPos[0], playerPos[1] + 1);
		break;
	case Direction::DOWN:
		target = mScene.getEnemyFromIndexPos(playerPos[0], playerPos[1] - 1);
		break;
	case Direction::RIGHT:
		target = mScene.getEnemyFromIndexPos(playerPos[0] + 1, playerPos[1]);
		break;
	case Direction::LEFT:
		target = mScene.getEnemyFromIndexPos(playerPos[0] - 1, playerPos[1]);
		break;
	}

	return target;
}

void IAttackProcess::createDamageText(EnemyComponent* target, int damageValue)
{
	//ダメージ値を生成する位置を計算
	XMFLOAT3 textPos = target->getPosition();
	textPos.y += 0.8f; //少しだけ上に

	//ダメージ値はビルボード処理でカメラの方向を向くようにするため、プレイヤーの前方ベクトルと右ベクトルを計算する
	XMFLOAT3 front = Math::rotateY(XMFLOAT3(0.0f, 0.0f, 1.0f), mPlayer.getRotation().y); //プレイヤーの前方ベクトル
	XMFLOAT3 right = Math::rotateY(front, -XM_PIDIV2);	//プレイヤーの右ベクトル
	textPos = textPos - front;	//敵オブジェクトより手前に

	int digit = 0;			//桁数
	int value = damageValue;		//表示したい数値
	std::vector<int> num(DamageTextGenerator::MaxNum);	//各桁の値, 最大数で配列を初期化しておく

	//桁数と各桁の値を取得
	if (value == 0) {
		num[0] = damageValue;
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

void IAttackProcess::moveNextTurn()
{
	mPlayer.turnEnd();
}

SingleAttackProcess::SingleAttackProcess(DungeonScene& scene, Player& player)
	: IAttackProcess(scene, player)
{
}

void SingleAttackProcess::execute()
{
	EnemyComponent* target = searchTargetEnemy();
	if (target == nullptr) return; //攻撃対象がいない場合は処理を終了

	int damage = calcDamage(target); //ダメージの計算
	target->giveDamage(damage); //ダメージを与える
	//ダメージエフェクト
	target->startFlash();	//敵を点滅させる
	createDamageText(target, damage);	//ダメージ値を描画
	mScene.getGame().getAudioManager().playSE("DAMAGE1");
	moveNextTurn();	//ターン経過
}

int SingleAttackProcess::calcDamage(EnemyComponent* target)
{
	int damage = max(mPlayer.getPower() - target->getDefence(), 0);
	return damage;
}


const float DoubleAttackProcess::NextAttackTime = 0.2f;	//2撃目の攻撃までの時間
DoubleAttackProcess::DoubleAttackProcess(DungeonScene& scene, Player& player)
	: IAttackProcess(scene, player)
{
	mTimer = 0.0f;
	mIsFirstAttackExecuted = false;
}

void DoubleAttackProcess::updateActor()
{
	if(mIsFirstAttackExecuted) mTimer += deltaTime;

	if (mTimer >= NextAttackTime) {
		executeSecondAttack();
		mTimer = 0.0f;
	}
}

void DoubleAttackProcess::execute()
{
	EnemyComponent* target = searchTargetEnemy();
	if (target == nullptr) return; //攻撃対象がいない場合は処理を終了

	//1撃目のダメージの計算
	int damage = calcDamage(target);
	target->giveDamage(damage); //ダメージを与える
	//ダメージエフェクト
	target->startFlash();	//敵を点滅させる
	createDamageText(target, damage);	//ダメージ値を描画
	mScene.getGame().getAudioManager().playSE("DAMAGE1");

	mIsFirstAttackExecuted = true;	//1撃目の攻撃が実行されたことを記録

}

int DoubleAttackProcess::calcDamage(EnemyComponent* target)
{
	int damage = max(mPlayer.getPower() - target->getDefence(), 0);
	return damage;
}

void DoubleAttackProcess::executeSecondAttack()
{
	EnemyComponent* target = searchTargetEnemy();

	if (target == nullptr) {
		mTimer = 0.0f;
		mIsFirstAttackExecuted = false;	//2撃目の攻撃が実行されたのでフラグをリセット
		moveNextTurn();	//攻撃対象がいない場合はターン経過
		return;
	}
	//2撃目のダメージの計算
	int damage = calcDamage(target);
	target->giveDamage(damage); //ダメージを与える
	//ダメージエフェクト
	const XMFLOAT3 effectPos = target->getPosition() - XMFLOAT3(0.0f, 0.1f, 0.0f);	//敵の少し前方にエフェクトを出す
	createDamageText(target, damage);	//ダメージ値を描画
	mScene.getGame().getAudioManager().playSE("DAMAGE1");

	mTimer = 0.0f;
	mIsFirstAttackExecuted = false;	//2撃目の攻撃が実行されたのでフラグをリセット
	moveNextTurn();	//ターン経過
}

AttackProcesses::AttackProcesses(DungeonScene& scene, Player& player)
{
	auto singleAttackProcess = std::make_unique<SingleAttackProcess>(scene, player);
	auto doubleAttackProcess = std::make_unique<DoubleAttackProcess>(scene, player);
	mAttackProcessMap[AttackType::SINGLE] = singleAttackProcess.get();
	mAttackProcessMap[AttackType::DOUBLE] = doubleAttackProcess.get();
	scene.addActor(std::move(singleAttackProcess));
	scene.addActor(std::move(doubleAttackProcess));
}

IAttackProcess* AttackProcesses::getAttackProcess(AttackType attackType)
{
	auto iter = mAttackProcessMap.find(attackType);
	if (iter != mAttackProcessMap.end()) {
		return iter->second;
	}

	return nullptr;
}
