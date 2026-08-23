#include "SimulationScene.h"
#include "Editer/GUIDebugger.h"
#include "CyanoSimulator.h"
#include "GameLoopCore/Game.h"

SimulationScene::SimulationScene(Game& game)
	: Scene(game)
{
}

void SimulationScene::onEnter()
{
	auto simulator = std::make_unique<CyanoSimulator>(*this);

#ifdef _DEBUG
	//mGame.getGUIDebugger().setCyanoSimulator(simulator.get());
#endif

	mSimulator = simulator.get();
	addActor(std::move(simulator));

#ifdef _DEBUG
	mDebugFlag = true;
#endif

}

void SimulationScene::onExit()
{
}

void SimulationScene::drawScene()
{
	mSimulator->draw();
}
