#pragma once
#include "Component.h"
#include "AttackType.h"
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

class PlayerAttackComponent : public Component
{
public:
	PlayerAttackComponent(DungeonScene& scene, Player& player);
	
	virtual void execute() = 0;

protected:
	DungeonScene& mScene;
	Player& mPlayer;

	EnemyComponent* searchTargetEnemy();
	virtual int calcDamage(EnemyComponent* target) = 0;
	void createDamageText(EnemyComponent* target, int damageValue);
	void moveNextTurn();

};

class PlayerSingleAttackComponent : public PlayerAttackComponent
{
public:
	PlayerSingleAttackComponent(DungeonScene& scene, Player& player);
	DECLARE_COMPONENT_NAME(PlayerSingleAttackComponent)
	void execute() override;
private:
	int calcDamage(EnemyComponent* target) override;
};

class PlayerDoubleAttackComponent : public PlayerAttackComponent
{
public:
	PlayerDoubleAttackComponent(DungeonScene& scene, Player& player);
	DECLARE_COMPONENT_NAME(PlayerDoubleAttackComponent)
	
	void updateComponent() override;
	void execute() override;
private:
	float mTimer;
	bool mIsFirstAttackExecuted;
	static const float NextAttackTime;

	int calcDamage(EnemyComponent* target) override;
	void executeSecondAttack();
};
