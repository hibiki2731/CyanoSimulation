#pragma once
#include "Actor.h"
#include "AssetManager.h"
#include "json_fwd.hpp"
#include <unordered_map>

struct ComponentLabel {
	std::string componentName;
	class Component* pComponent;
};

class Object : public Actor
{
public:
	Object(Scene& scene, const std::string& name, const std::string& meshID, float x = 0.0f, float z = 0.0f);
	Object(Scene& scene, const std::string& objectID);

	void endProcessActor() override;

	const std::string getClassName() const override {
		return "Object";
	}
	//オブジェクトの名前を返す
	const std::string& getName() { return mName; }

	//オブジェクト名の設定
	void setName(const std::string& name) { mName = name; }
	//コンポーネントのラベル付け
	void addComponentLabel(const std::string& labelName, const std::string& componentName);
	virtual void applyComponentLabel() {};

protected:
	std::unordered_map<std::string, ComponentLabel> mComponentLabels;

private:
	void loadActorData(nlohmann::json& json);
	void loadComponentData(nlohmann::json& json);
	void loadLabeData(nlohmann::json& json);


	std::string mName;
#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};

