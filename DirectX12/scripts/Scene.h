#pragma once
#include <string>

class SceneManager;

class Scene
{
public:
	Scene(SceneManager* sceneManager) : mSceneManager(sceneManager) {};
	virtual ~Scene() {};

	//シーンごとの振る舞い
	virtual void fastUpdate() {};
	virtual void update() {};
	virtual void input() {};

	virtual void onEnter() = 0;
	virtual void onExit() = 0;

	//シーン名の取得
	virtual const std::string& getName() const = 0;
protected:
	SceneManager* mSceneManager;
};

