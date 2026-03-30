#pragma once
#include <string>
#include <unordered_map>
#include "Actor.h"

class Game;

enum class SceneType {
	TITLE,
	TOWN,
	MAP,
};

class SceneManager : public Actor
{
public:
	SceneManager(Game& game);

	void fastUpdateActor() override;
	void updateActor() override;
	void inputActor() override;


	const std::string& getCurrentSceneType();

	void transitToTitle();
	void transitToTown();
	void transitToMap();
	void transitScene();

private:
	std::string mNextSceneType;
	std::string mCurrentSceneType;
	class Scene* mCurrentScene;
	std::unordered_map<std::string, std::unique_ptr<Scene>> mSceneMap;
};

