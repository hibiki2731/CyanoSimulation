#pragma once
#include "Scene.h"
class SimulationScene :
    public Scene
{
public:
    SimulationScene(Game& game);

	void onEnter() override;
	void onExit() override;

	void drawScene() override;
	const std::string getName() const override { return "SIMULATION"; }

private:
	class CyanoSimulator* mSimulator;
};

