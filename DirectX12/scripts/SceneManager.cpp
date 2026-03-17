#include "SceneManager.h"
#include "Game.h"
#include "MapManager.h"

SceneManager::SceneManager(Game* game)
{
	mCurrentScene = SceneType::TITLE;
	mNextScene = mCurrentScene;
	mGame = game;
}

void SceneManager::setScene(SceneType scene)
{
	mCurrentScene = scene;
}

SceneType SceneManager::getCurrentScene()
{
	return mCurrentScene;
}

void SceneManager::transitToTitle()
{
	mNextScene = SceneType::TITLE;
}

void SceneManager::transitToTown()
{
	mNextScene = SceneType::TOWN;
}

void SceneManager::transitToMap()
{
	mNextScene = SceneType::MAP;
}

void SceneManager::transitScene()
{
	if (mNextScene != mCurrentScene) {
		mGame->clearActors();

		switch (mNextScene) {
		case SceneType::TITLE:
			mCurrentScene = SceneType::TITLE;
			break;
		case SceneType::TOWN:
			mCurrentScene = SceneType::TOWN;
			break;
		case SceneType::MAP:
			mGame->getMapManager()->createMap();
			mCurrentScene = SceneType::MAP;
			break;
		}

	}
}

