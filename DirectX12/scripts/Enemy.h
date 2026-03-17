#pragma once
#include "Actor.h"
#include "MapManager.h"
#include "json.hpp"
#include  "magic_enum.hpp"

class EnemyComponent;

enum class MeshName;
enum class MovePattern;

class Enemy : public Actor
{
public:
	Enemy(Game* game, CharacterType::Type type, float x, float y);

	void updateActor() override;
	void inputActor() override;

private:

	struct EnemyParam {
		int hp;
		int power;
		int defense;
		MeshName meshName;
		MovePattern movePattern;
		int senseRange;
	};
	class EnemyComponent* mEnemy;
};

