#pragma once
#include <string>
#include <unordered_map>

class Game;

enum class SceneType {
	TITLE,
	TOWN,
	MAP,
};

class SceneManager
{
public:
	SceneManager(Game& game);

	//シーンの更新
	void fastUpdateScene();
	void updateScene();
	void lateUpdateScene();
	//シーンの入力
	void inputScene();

	//シーン中のアクター配列を更新
	void joinSceneActors();

	//シーン中のアクターの削除
	void removeSceneActors();


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

