#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Main.h"
#include "SimulationScene.h"
#include "GameLoopCore/Game.h"
#include "Scene/SceneManager.h"
#include <filesystem>

INT WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT) {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::filesystem::path absPath = std::filesystem::absolute("root");

	auto game = std::make_unique<Game>();
	game->init();
	auto& sceneManager = game->getSceneManager();
	sceneManager.addScene("Simulation", std::move(std::make_unique<SimulationScene>(*game.get())));
	sceneManager.setNextScene("Simulation");

	game->runLoop();

	return game->endProcess();
}
