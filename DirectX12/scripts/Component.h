#pragma once
#include <memory>
#include <vector>
#include <concepts>
#include <algorithm>

//前方宣言
class Actor;

class Component
{
public:
	Component(Actor* owner, int updateOrder = 100);
	virtual ~Component();

	//入力
	virtual void inputComponent() {};

	//更新
	virtual void updateComponent();
	int getUpdateOrder() const { return mUpdateOrder; }

	//終了処理
	virtual void endProccess() {};

protected:
	//所有アクター
	Actor* mOwner;
	//更新順序
	int mUpdateOrder;

};
