#pragma once

class Game;

enum class SceneType {
	TITLE,
	TOWN,
	MAP,
};

class SceneManager
{
public:
	SceneManager(Game* game);
	void setScene(SceneType scene);
	SceneType getCurrentScene();

	void transitToTitle();
	void transitToTown();
	void transitToMap();
	void transitScene();

private:
	SceneType mNextScene;
	SceneType mCurrentScene;
	Game* mGame;
};

