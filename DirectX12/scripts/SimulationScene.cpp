#include "SimulationScene.h"
#include "GUIDebugger.h"
#include "CyanoSimulator.h"

SimulationScene::SimulationScene(Game& game)
	: Scene(game)
{
}

void SimulationScene::onEnter()
{
	auto simulator = std::make_unique<CyanoSimulator>(*this);
	addActor(std::move(simulator));

#ifdef _DEBUG
	mDebugFlag = true;
#endif

}

void SimulationScene::onExit()
{
}
