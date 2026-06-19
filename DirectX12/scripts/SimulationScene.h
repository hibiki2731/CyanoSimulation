#pragma once
#include "Scene.h"
class SimulationScene :
    public Scene
{
public:
    SimulationScene(Game& game);

	void onEnter() override;
	void onExit() override;
	const std::string getName() const override { return "SIMULATION"; }
};

