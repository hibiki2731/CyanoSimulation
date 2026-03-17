#pragma once
#include "Component.h"
#include "MapManager.h"
#include <DirectXMath.h>

using namespace DirectX;


class CharacterComponent : public Component
{
public:
	CharacterComponent(Actor* owner, int updateOrder = 100);
	~CharacterComponent();

	void inputComponent() override;
	void updateComponent() override;
	void endProccess() override;

	//getter
	int getHP();
	int getMaxHP();
	int getPower();
	int getDefense();
	int getDirection();
	bool getAlive();
	std::vector<int>& getIndexPos();
	int getIndexPosInt();
	MapManager* getMapManager();

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

	MapManager* mMapManager;
};

