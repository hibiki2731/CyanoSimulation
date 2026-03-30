#include <iostream>
#include "SceneManager.h"
#include "Game.h"
#include "MapManager.h"
#include "TownManager.h"
#include "DungeonScene.h"

SceneManager::SceneManager(Game& game) : Actor(game)
{
	mCurrentSceneType = "TOWN";
	mNextSceneType = mCurrentSceneType;
	
	//シーンの登録
	mSceneMap["TOWN"] = std::make_unique<TownManager>(game, this);
	mSceneMap["DUNGEON"] = std::make_unique<DungeonScene>(game, this);
	mCurrentScene = mSceneMap["TOWN"].get();
	mCurrentScene->onEnter();
}

void SceneManager::fastUpdateActor()
{
	if (mCurrentScene)
		mCurrentScene->fastUpdate();
}

void SceneManager::updateActor()
{
	if (mCurrentScene)
		mCurrentScene->update();
}

void SceneManager::inputActor()
{
	if (mCurrentScene)
		mCurrentScene->input();
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

