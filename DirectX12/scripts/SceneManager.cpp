#include <iostream>
#include "SceneManager.h"
#include "Game.h"
#include "MapManager.h"
#include "TownManager.h"
#include "DungeonScene.h"

SceneManager::SceneManager(Game& game)
{
	mCurrentSceneType = "TOWN";
	mNextSceneType = mCurrentSceneType;
	
	//シーンの登録
	mSceneMap["TOWN"] = std::make_unique<TownManager>(game);
	mSceneMap["DUNGEON"] = std::make_unique<DungeonScene>(game, this);
	mCurrentScene = mSceneMap["TOWN"].get();
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

void SceneManager::joinSceneActors()
{
	if (mCurrentScene)
		mCurrentScene->joinActors();
}

void SceneManager::removeSceneActors()
{
	if (mCurrentScene)
		mCurrentScene->removeActors();
}

const std::string& SceneManager::getCurrentSceneType()
{
	return mCurrentSceneType;
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

void SceneManager::transitScene()
{
	if (mNextSceneType != mCurrentSceneType) {
	
		//シーンから出る処理
		if (mCurrentScene) {
			mCurrentScene->onExit();
		}
		
		//新たなシーンを取得
		auto iter = mSceneMap.find(mNextSceneType);
		if (iter != mSceneMap.end()) {

			mCurrentSceneType = mNextSceneType;
			mCurrentScene = iter->second.get();
			mCurrentScene->onEnter();
		}
		else {
			std::cerr << "シーンが存在しません" << std::endl;
			mCurrentScene = nullptr;
		}
	}
}

