#pragma once
#include "Actor.h"
#include "AssetManager.h"

class Object : public Actor
{
public:
	Object(Scene& scene, const std::string& name, const std::string& meshID, float x = 0.0f, float z = 0.0f);
	Object(Scene& scene, const std::string& name);

	//オブジェクトの名前を返す
	const std::string getClassName() const override {
		return mName;
	}
private:
	std::string mName;
#ifdef _DEBUG
	friend class GUIDebugger;
#endif
};

