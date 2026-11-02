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
	//_CrtSetBreakAlloc(69732);
//	_CrtSetBreakAlloc(69731);
	//_CrtSetBreakAlloc(69701);
	//_CrtSetBreakAlloc(69676);
	//_CrtSetBreakAlloc(69675);
	//_CrtSetBreakAlloc(69674);
	//_CrtSetBreakAlloc(69647);
	//_CrtSetBreakAlloc(69634);
	//_CrtSetBreakAlloc(69633);
	//_CrtSetBreakAlloc(69632);
	//_CrtSetBreakAlloc(69604);


	auto game = std::make_unique<Game>();
	game->init();
	auto& sceneManager = game->getSceneManager();
	sceneManager.addScene("Simulation", std::move(std::make_unique<SimulationScene>(*game.get())));
	sceneManager.setNextScene("Simulation");

	game->runLoop();

	return game->endProcess();
}
