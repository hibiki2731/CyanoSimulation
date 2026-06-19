#include <iostream>
#include "SceneManager.h"
#include "Game.h"
#include "json.hpp"
#include "AssetManager.h"
#include "Scene.h"
#include "Object.h"
#include "SimulationScene.h"

SceneManager::SceneManager(Game& game)
	:mGame(game)
{
	mCurrentSceneType = "SIMULATION";
	mNextSceneType = mCurrentSceneType;

	mSceneMap["SIMULATION"] = std::make_unique<SimulationScene>(game);
	
	//シーンの登録
	mCurrentScene = mSceneMap[mCurrentSceneType].get();
	mCurrentScene->onEnter();
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

const std::string& SceneManager::getCurrentSceneType()
{
	return mCurrentSceneType;
}

Scene& SceneManager::getCurrentScene()
{
	return *mCurrentScene;
}

void SceneManager::transitToTitle()
{
	mNextSceneType = "TITLE";
}

void SceneManager::transitToTown()
{
	mNextSceneType = "TOWN";
}

void SceneManager::transitToMap()
{
	mNextSceneType = "DUNGEON";
}

void SceneManager::transitToGameOver(){
	mNextSceneType = "GAME_OVER";
}

void SceneManager::transitScene()
{
	if (mNextSceneType != mCurrentSceneType) {
	
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
}

#ifdef _DEBUG
void SceneManager::drawDebugGUI()
{
	mCurrentScene->drawDebugGUI();
}
#endif
