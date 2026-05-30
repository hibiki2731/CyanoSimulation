#pragma once
#include "Component.h"
#include <DirectXMath.h>

using namespace DirectX;

class DungeonScene;

class CharacterComponent : public Component
{
public:
	CharacterComponent(Actor& owner, DungeonScene& scene);
	DECLARE_COMPONENT_NAME(CharacterComponent)

	//getter
	int getHP();
	int getMaxHP();
	int getPower();
	int getDefence();
	int getDirection();
	bool getAlive();
	std::vector<int>& getIndexPos();
	int getIndexPosInt();

	//setter
	void setMaxHP(int maxHP);
	void setHP(int hp);
	void setPower(int power);
	void setDefense(int defense);
	void setDirection(int direction);
	void setIndexPos(int x, int y);
	void setIndexPosInt(int indexPos);

	void addHP(int hp);

	void giveDamage(int damage);
	void turnRight();
	void turnLeft();


protected:
	//ステータス
	int mMaxHP;
	int mHP;
	int mPower;
	int mDefense;
	bool isAlive;

	//向き
	int mDirection; //左上右下 0000

	//位置
	std::vector<int> mIndexPos; //{x, y}

	DungeonScene& mScene;
};

