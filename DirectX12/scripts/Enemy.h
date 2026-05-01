#pragma once
#include "Actor.h"
#include <string>

class EnemyComponent;

enum class MeshName;
enum class MovePattern;

class Enemy : public Actor
{
public:
	Enemy(class DungeonScene& scene, const std::string& enemyID, float x, float y);
	DECLARE_CLASS_NAME(Enemy)

private:

	struct EnemyParam {
		int hp;
		int power;
		int defense;
		std::string meshID;
		MovePattern movePattern;
		int senseRange;
		int dropMoney;
	};

	class EnemyComponent* mEnemy;
};

