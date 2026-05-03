#pragma once
#include <memory>
#include <vector>
#include <concepts>
#include <algorithm>
#include <string>
#include "json_fwd.hpp"
#define DECLARE_COMPONENT_NAME(componentName) \
			const std::string getComponentName() const override {return std::string(#componentName); }

//前方宣言
class Actor;

class Component
{
public:
	Component(Actor& owner, int updateOrder = 100);
	virtual ~Component() {};
	virtual const std::string getComponentName() const = 0;
	
	//jsonからのデータ取得
	virtual void loadFromJson(const nlohmann::json& json) {};

	//入力
	virtual void inputComponent() {};

	//更新
	virtual void fastUpdateComponent() {};
	virtual void updateComponent() {};
	virtual void lateUpdateComponent() {};
	int getUpdateOrder() const { return mUpdateOrder; }

	//終了処理
	virtual void endProcess() {};

protected:
	//所有アクター
	Actor& mOwner;
	//更新順序
	int mUpdateOrder;

#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};