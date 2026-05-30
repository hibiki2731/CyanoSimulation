#pragma once
#include "Actor.h"
#include <unordered_map>
//実装必要な機能
//コンストラクタ in:シーンポインタ、プレイヤーポインタ
//索敵
//ダメージ計算　　out ：ダメージの値  in:敵ポインタ、プレイヤーポインタ、
//ダメージテキストの生成 out:ダメージテキストの生成 in:ダメージの値、敵の位置	
//SEの再生　
//敵の点滅
//ターンの経過  
class DungeonScene;
class Player;
class EnemyComponent;

enum class AttackType {
	SINGLE,
	DOUBLE,
};

class IAttackProcess : public Actor
{
public:
	IAttackProcess(DungeonScene& scene, Player& player);
	
	virtual void execute() = 0;

protected:
	DungeonScene& mScene;
	Player& mPlayer;

	EnemyComponent* searchTargetEnemy();
	virtual int calcDamage(EnemyComponent* target) = 0;
	void createDamageText(EnemyComponent* target, int damageValue);
	void moveNextTurn();

};

class SingleAttackProcess : public IAttackProcess
{
public:
	SingleAttackProcess(DungeonScene& scene, Player& player);
	DECLARE_CLASS_NAME(SINGLEATTACKPROCESS)
	void execute() override;
private:
	int calcDamage(EnemyComponent* target) override;
};

class DoubleAttackProcess : public IAttackProcess
{
public:
	DoubleAttackProcess(DungeonScene& scene, Player& player);
	DECLARE_CLASS_NAME(DOUBLEATTACKPROCESS)
	
	void updateActor() override;
	void execute() override;
private:
	float mTimer;
	bool mIsFirstAttackExecuted;
	static const float NextAttackTime;

	int calcDamage(EnemyComponent* target) override;
	void executeSecondAttack();
};


//各攻撃処理のクラスを変更するためのクラス
class AttackProcesses {
public:
	AttackProcesses(DungeonScene& scene, Player& player);
	IAttackProcess* getAttackProcess(AttackType attackType);

private:
	std::unordered_map<AttackType, IAttackProcess*> mAttackProcessMap;
};
