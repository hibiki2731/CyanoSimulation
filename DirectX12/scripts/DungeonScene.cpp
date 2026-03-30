#include "DungeonScene.h"
#include "MapManager.h"
#include "Game.h"
#include "SceneManager.h"

DungeonScene::DungeonScene(Game& game, SceneManager* sceneManager)
	:Scene(sceneManager)
{
	mMapManager = std::make_unique<MapManager>(game, sceneManager);
}

void DungeonScene::fastUpdate() {
	mMapManager->updateTurn();
}

void DungeonScene::update()
{
}

void DungeonScene::onEnter()
{
	mMapManager->begin();
}

void DungeonScene::onExit()
{
	mMapManager->end();
}


