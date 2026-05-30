#pragma once
#include "Actor.h"
#include "Object.h"
#include "Definition.h"
#include "AttackType.h"
#include "json_fwd.hpp"
#include <vector>

//前方宣言
class CameraComponent;
class CharacterComponent;
class MapGenerator;
class PlayerManager;
class ItemManager;
class DungeonScene;

//PlayerManagerがプレイヤーデータを管理し、Playerクラスはダンジョンシーン中のプレイヤーの位置やHPなどのステータス、行動処理を管理する
class Player : public Actor
{
public:
	Player(DungeonScene& scene, float x, float y);
	DECLARE_CLASS_NAME(PLAYER)

	void inputActor() override;			//入力処理
	void updateActor() override;		//毎フレームの更新処理
	void endProcessActor() override;	//コンポーネントが破棄される際の処理

	//getter
	int getDirection();
	void getIndexPos(int(&pos)[2]);
	std::vector<int> getIndexPos();
	int getCurrentHP();
	int getCurrentAP();
	int getPower();
	int getDefence();
	int getSelectItemIndex();
	bool getIsActing();
	const std::string& getSelectItemID();

	//setter
	void setIndexPos(int x, int y);

	//プレイヤーにダメージを与える
	void giveDamage(int damage);

	//ターン経過処理
	void startAct();
	void moveToEnemyTurn();
	void endAct();
private:
	//--ターン経過する行動--
	void attack();
	void move(Direction direction);
	void collect();
	void useItem();
	void getTreasure();

	void rotate(Direction direction);	//視点移動
	void damagedProcess();				//ダメージを受けたときの処理
	void updateFlash();					//点滅処理の更新	
	void selectNextItem();				//アイテム選択
	void selectPreviousItem();			//アイテム選択
	void moveNextFloor();				//次の階へ移動

	float	 mMoveSpeed;		//移動速度
	float	 mRotSpeed;			//回転速度
	XMFLOAT3 mTargetPos;		//移動先の座標
	XMFLOAT3 mTargetRot;		//視点移動先の回転角
	bool	 isActing;			//true : ターン経過する行動を実行中, false : 何も行動していない
	bool	 isRotating;		//true : 視点移動中, false : 視点移動していない
	int		 mPendingDamage;	//次の更新でHPから引かれるダメージ量
	float	 mFlashTimer;		//ダメージを受けたときの点滅用タイマー
	float	 mFlashDuration;	//ダメージを受けたときの点滅時間
	int		 mSelectItemIndex;	//現在選択しているアイテムのインデックス
	int		 mAP;				//現在のAP

	//コンポーネント
	CameraComponent* mCamera;
	CharacterComponent* mCharacter;

	//データ管理クラスやシーンへの参照
	ItemManager& mItemManager;
	PlayerManager& mPlayerManager;
	DungeonScene& mScene;
	const struct PlayerData& mPlayerData;	//プレイヤーデータの参照

	//移動用コンポーネント
	class PlayerMoveComponent* mMoveComponent;
	//攻撃処理用コンポーネント
	std::unordered_map<AttackType, class PlayerAttackComponent*> mAttackComponents;
};

class TreasureWindow : public Object
{
public:
	TreasureWindow(Scene& scene, const std::string& itemID);
	DECLARE_CLASS_NAME(TREASUREWINDOW)

	void inputActor() override;			//入力処理
};
