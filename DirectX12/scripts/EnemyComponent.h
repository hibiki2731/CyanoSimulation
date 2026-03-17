#pragma once
#include <random>
#include <queue>
#include "CharacterComponent.h"
#include "MeshComponent.h"

class Player;

enum class MovePattern{
	RANDOM,
	ASTAR,
	SENSE,
};

class EnemyComponent : public CharacterComponent
{
public:
	EnemyComponent(Actor* owner, int updateOrder = 100);

	void inputComponent() override;
	void updateComponent() override;
	void endProccess() override;

	//アクティブな時のみ行う。プレイヤーに近い敵から更新される
	void updateActiveProcess();

	void setMesh(MeshComponent* mesh);
	void setEnemyTtype(int type);
	void setMovePattern(MovePattern state);
	void setSenseRange(int range);
	void activate();

	void startFlash();

	//getter
	XMFLOAT3 getPosition();
	int getDist();

private:
	//A*アルゴリズム用構造体
	struct Node {
		int x, y;
		int fCost;

		bool operator>(const Node& other)const {
			return fCost > other.fCost;
		}
	};
	struct Cell {
		int gCost = 999999; //初期値は大きな値にする
		bool isClosed = false; //探索済みか判定

		//親ノードの位置
		int parentX = -1;
		int parentY = -1;

	};

	void updateFlash(); //ダメージ時の点滅を更新
	void move();
	void attack();
	void finishAct();
	void calcTargetIndex(int(&targetIndex)[2]);
	void Astar(int (& targetIndex)[2]);
	void randomWalk(int(&targetIndex)[2]);

	//アクターに設定されているメッシュ
	MeshComponent* mMesh;

	//ダメージ時の点滅
	float mFlashTimer;
	float mFlashDuration;

	//移動用
	XMFLOAT3 mTargetPos;
	float mMoveSpeed;
	bool isMoving;

	//敵タイプ
	int mEnemyType;

	//行動パターン
	MovePattern mState;
	int mSenseRange; //プレイヤーを感知する範囲(マンハッタン距離)
	
	//ターン制御
	bool isActive;

	int mDistPlayer; //プレイヤーからの距離(マンハッタン距離)

};

