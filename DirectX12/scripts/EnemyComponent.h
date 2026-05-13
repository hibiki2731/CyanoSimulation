#pragma once
#include <random>
#include <queue>
#include "CharacterComponent.h"
#include "MeshComponent.h"

//前方宣言
class Player;
class DungeonScene;

//---エネミーの行動パターン---
//RANDOM	: ランダムに移動
//ASTAR		: A*アルゴリズムでプレイヤーを追跡
//SENSE		: プレイヤーとの距離が一定値より小さくなったらA*で追跡
enum class MovePattern{
	RANDOM,
	ASTAR,
	SENSE,
};

class EnemyComponent : public CharacterComponent
{
public:
	EnemyComponent(Actor& owner, DungeonScene& scene);
	DECLARE_COMPONENT_NAME(EnemyComponent)

	void updateComponent() override;	//毎フレームの更新処理
	void endProcess() override;			//コンポーネントが破棄される際の処理
	void startAct();					//エネミーの行動を開始する
	void startFlash();					//ダメージを受けたときの点滅を開始

	//setter
	void setMesh(MeshComponent* mesh);		//メッシュの設定
	void setMovePattern(MovePattern state); //行動パターンの設定
	void setSenseRange(int range);			//行動パターンがSENSEの場合の索敵範囲を設定
	void setDropMoney(int money);			//エネミー撃破時に落とすお金の量を設定
	void setName(const std::string& name);	//エネミーの名前を設定

	//getter
	XMFLOAT3 getPosition();	//位置の取得
	int getDist();			//プレイヤーとの距離を取得
	const std::string& getName() { return mName; } //エネミーの名前を取得

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

	void attack();									//プレイヤーが隣接していたら攻撃する
	void move();									//移動のためのデータ更新を行う
	void calcTargetIndex(int(&targetIndex)[2]);		//移動先を計算
	void Astar(int (& targetIndex)[2]);				//A*アルゴリズムで移動先を決定
	void randomWalk(int(&targetIndex)[2]);			//ランダムに移動先を決定
	void updateFlash(); 							//ダメージ時の点滅を更新
	void finishAct();								//行動の終了処理を行う
	void deadProcess();								//死亡時の処理

	std::string		mName; 				//エネミーの名前
	MeshComponent*	mMesh;			    //エネミーに設定されているメッシュ
	int				mDropMoney;			//ドロップするお金
	bool			isActive;			//true : 行動可能  false : 行動済み
	MovePattern		mState;				//行動パターン
	int				mSenseRange; 		//行動パターンがSENSEだった場合のプレイヤーを感知する範囲(マンハッタン距離)
	int				mDistPlayer; 		//プレイヤーからの距離(マンハッタン距離)、エネミーの更新順序を計算する際に使用
	DungeonScene&	mScene;				//シーン
	//---点滅処理用---
	float		    mFlashTimer;		//ダメージ時の点滅用タイマー
	float			mFlashDuration;		//ダメージ時の点滅の長さ
	//--移動処理用---
	XMFLOAT3		mTargetPos;			//移動先の位置
	float			mMoveSpeed;			//移動スピード
	bool			isMoving;			//移動フラグ　ture : 移動中　false : 停止
	
	const struct PlayerData& mPlayerData;//プレイヤー情報
};

