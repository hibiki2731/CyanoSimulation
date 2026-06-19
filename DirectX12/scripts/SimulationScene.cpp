#include "SimulationScene.h"
#include "GUIDebugger.h"

SimulationScene::SimulationScene(Game& game)
	: Scene(game)
{
}

void SimulationScene::onEnter()
{
#ifdef _DEBUG
	mDebugFlag = true;
#endif

}

void SimulationScene::onExit()
{
}
