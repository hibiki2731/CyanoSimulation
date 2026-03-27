#pragma once
#include "Actor.h"
#include <string>

class EnemyComponent;

enum class MeshName;
enum class MovePattern;

class Enemy : public Actor
{
public:
	Enemy(Game* game, const std::string& enemyID, float x, float y);

	void updateActor() override;
	void inputActor() override;

private:

	struct EnemyParam {
		int hp;
		int power;
		int defense;
		std::string meshName;
		MovePattern movePattern;
		int senseRange;
	};
	class EnemyComponent* mEnemy;
};

