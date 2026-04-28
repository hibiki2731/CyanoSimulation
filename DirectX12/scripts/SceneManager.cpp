#include <iostream>
#include "SceneManager.h"
#include "Game.h"
#include "TownScene.h"
#include "DungeonScene.h"
#include "GameOverScene.h"
#include "TitleScene.h"

SceneManager::SceneManager(Game& game)
{
	mCurrentSceneType = "TITLE";
	mNextSceneType = mCurrentSceneType;
	
	//シーンの登録
	mSceneMap["TOWN"] = std::make_unique<TownScene>(game);
	mSceneMap["DUNGEON"] = std::make_unique<DungeonScene>(game);
	mSceneMap["GAME_OVER"] = std::make_unique<GameOverScene>(game); 
	mSceneMap["TITLE"] = std::make_unique<TitleScene>(game);
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
