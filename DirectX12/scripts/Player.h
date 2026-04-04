#pragma once
#include "Actor.h"
#include "Definition.h"
#include "json.hpp"
#include <vector>

class CameraComponent;
class CharacterComponent;
class MapGenerator;
class PlayerManager;
class ItemManager;
class DungeonScene;

class Player : public Actor
{
public:
	Player(DungeonScene& scene, float x, float y);

	void inputActor() override;
	void updateActor() override;
	void endProcessActor() override;

	int getDirection();
	void getIndexPos(int(&pos)[2]);
	int getMaxHP();
	int getHP();
	int getPower();
	int getDefense();
	int getAP();
	int getMaxAP();
	int getStorageSize();

	//プレイヤーにダメージを与える
	void giveDamage(int damage);



private:
	void attack();
	void calcDamageText(const XMFLOAT3& targetPos, int val);
	void move(Direction direction);
	void rotate(Direction direction);
	void calcMoveDirectionToIndexPos(Direction moveDirection, int (& indexPos)[2]);
	void collect();
	void damagedProcess();
	void updateFlash();
	void useItem();
	void turnEnd();

	float mMoveSpeed;
	float mRotSpeed;

	//移動処理用
	XMFLOAT3 mTargetPos;
	XMFLOAT3 mTargetRot;
	bool isActing;
	bool isRotating;

	//ダメージ処理
	int mPendingDamage;
	float mFlashTimer;
	float mFlashDuration;

	//アイテム
	int mSelectItemIndex;
	int mStorageSize;

	//行動回数制限
	int mAP;
	int mMaxAP;


	CameraComponent* mCamera;
	CharacterComponent* mCharacter;
	ItemManager& mItemManager;
	PlayerManager& mPlayerManager;
	DungeonScene& mScene;
};

