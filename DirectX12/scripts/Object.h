#pragma once
#include "Actor.h"
#include "AssetManager.h"
#include "json_fwd.hpp"
#include <unordered_map>

//ラベル付けしたいコンポーネントを設定し、JSONから読み込むための構造体
struct ComponentLabel{
	std::string		 componentName;		//ラベル付けしたコンポーネントの型名　この情報から設定できるコンポーネントを絞る
	class Component* pComponent;		//参照するコンポーネント
};

class Object : public Actor
{
public:
	//メッシュのみを生成する場合
	Object(Scene& scene, const std::string& name, const std::string& meshID, float x = 0.0f, float z = 0.0f);
	//オブジェクトIDから生成する
	Object(Scene& scene, const std::string& objectID);

	//クラスの名前を取得
	const std::string getClassName() const override {
		return "Object";
	}
	//アクターの終了処理
	void endProcessActor() override;

	//オブジェクトの名前を返す
	const std::string& getName() { return mName; }
	//オブジェクト名の設定
	void setName(const std::string& name) { mName = name; }

	//コンポーネントのラベル付け
	void addComponentLabel(const std::string& labelName, const std::string& componentName);
	//派生先のクラスでラベル付けしたコンポーネントを参照するメンバー変数を設定する
	virtual void applyComponentLabel() {};

protected:
	//ラベル付けしたコンポーネントのポインタと型名をラベル名から取得するためのmap
	std::unordered_map<std::string, ComponentLabel> mComponentLabels;

private:
	void loadActorData(nlohmann::json& json);
	void loadComponentData(nlohmann::json& json);
	void loadLabelData(nlohmann::json& json);

	std::string mName;
#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};

