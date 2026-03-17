#pragma once
#include <concepts>
#include <memory>
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

//前方宣言
class Game;
class Component;

class Actor
{
public:

	enum State {
		Active,
		Paused,
		Dead
	};

	Actor(Game* game);
	Actor(Game* game, float x, float y);

	virtual ~Actor();

	//入力処理
	void input();
	virtual void inputActor() {};

	//更新関数
	void update();
	//コンポーネントを更新
	void updateComponents();
	//アクター独自の更新処理
	virtual void updateActor() {};

	//コンポーネントの追加/削除
	void addComponent(std::unique_ptr<Component> component);
	void removeComponent(std::unique_ptr<Component>& component);

	//終了処理
	void endProccess();

	//セッター
	void setState(State state);
	void setPosition(const XMFLOAT3& position);
	void setScale(const XMFLOAT3& scale);
	void setRotation(const XMFLOAT3& rotation);

	void setXPos(float x);
	void setYPos(float y);
	void setZPos(float z);
	void setXRot(float x);
	void setYRot(float y);
	void setZRot(float z);

	void movePositon(const XMFLOAT3& diff);

	//ゲッター
	State getState();
	XMFLOAT3 getPosition() const;
	XMFLOAT3 getScale() const;
	XMFLOAT3 getRotation() const;
	Game* getGame();

protected:
	//アクターの状態
	State mState;

	//コンポーネント
	std::vector<std::unique_ptr<Component>> mComponents;
	Game* mGame;

	//ベクトル
	XMFLOAT3 mPosition;
	XMFLOAT3 mScale;
	XMFLOAT3 mRotation;
};
