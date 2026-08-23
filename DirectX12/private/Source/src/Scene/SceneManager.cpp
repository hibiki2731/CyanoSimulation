#include <iostream>
#include "Scene/SceneManager.h"
#include "GameLoopCore/Game.h"
#include <json.hpp>
#include "Memory/AssetManager.h"
#include "Scene/Scene.h"
#include "Editer/Object.h"

SceneManager::SceneManager(Game& game)
	:mGame(game)
{
	mCurrentSceneType = "";
	mNextSceneType = mCurrentSceneType;

	
	//シーンの登録
	mCurrentScene = nullptr;
}

void SceneManager::fastUpdateScene()
{
	if (mCurrentScene)
		mCurrentScene->fastUpdate();
}

void SceneManager::updateScene()
{
	if (mCurrentScene)
		mCurrentScene->update();
}

void SceneManager::lateUpdateScene()
{
	if (mCurrentScene)
		mCurrentScene->lateUpdate();
}

void SceneManager::inputScene()
{
	if (mCurrentScene)
		mCurrentScene->input();
}

void SceneManager::drawScene3D()
{
	if (mCurrentScene) {
		mCurrentScene->draw3D();
	}
		
}

void SceneManager::drawScene2D()
{
	if (mCurrentScene) {
		mCurrentScene->draw2D();
	}
}

void SceneManager::drawScene()
{
	if (mCurrentScene)
		mCurrentScene->drawScene();
}

void SceneManager::joinSceneActors()
{
	if (mCurrentScene)
		mCurrentScene->joinActors();
}

void SceneManager::removeSceneActors()
{
	if (mCurrentScene) {
		mCurrentScene->removeActors();
	}
}

Scene& SceneManager::getCurrentScene()
{
	return *mCurrentScene;
}

void SceneManager::addScene(const std::string& sceneName, std::unique_ptr<Scene>& scene)
{
	mSceneMap.emplace(sceneName, std::move(scene));
}

void SceneManager::setNextScene(const std::string& nextSceneType) {
	mNextSceneType = nextSceneType;
}

void SceneManager::transitScene()
{
	if (mNextSceneType == mCurrentSceneType) return;

	//シーンから出る処理
	if (mCurrentScene) {
		mCurrentScene->onExit();
		mCurrentScene->refreshActors();
		removeSceneActors();
	}

	//新たなシーンを取得
	auto iter = mSceneMap.find(mNextSceneType);
	if (iter != mSceneMap.end()) {

		mCurrentSceneType = mNextSceneType;
		mCurrentScene = iter->second.get();
		mCurrentScene->onEnter();
		mCurrentScene->createObjects();
	}
	else {
		std::cerr << "シーンが存在しません" << std::endl;
		mCurrentScene = nullptr;
	}
}

#ifdef _DEBUG
void SceneManager::drawDebugGUI()
{
	mCurrentScene->drawDebugGUI();
}
#endif
