#include "SimulationScene.h"
#include "GUIDebugger.h"
#include "CyanoSimulator.h"
#include "Game.h"

SimulationScene::SimulationScene(Game& game)
	: Scene(game)
{
}

void SimulationScene::onEnter()
{
	auto simulator = std::make_unique<CyanoSimulator>(*this);

#ifdef _DEBUG
	mGame.getGUIDebugger().setCyanoSimulator(simulator.get());
#endif

	addActor(std::move(simulator));

#ifdef _DEBUG
	mDebugFlag = true;
#endif

}

void SimulationScene::onExit()
{
}
