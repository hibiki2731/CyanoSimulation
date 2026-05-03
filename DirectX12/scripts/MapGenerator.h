#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Random.h"

class Game;
class Player;
class SceneManager;
class MiniMap;
class EnemyComponent;
class Resource;
class DungeonScene;
enum class Stage;
enum class TurnType;

class MapGenerator
{
public:
	MapGenerator(DungeonScene& scene);
	~MapGenerator() {};

	void begin();
	void end();

	void createMap();

private:

	void loadMap(Stage stage);
	void createTile();
	void createCharacter();

#ifdef _DEBUG
	//DEBUG用
#endif

	//ダンジョンシーン	
	DungeonScene& mScene;
};

