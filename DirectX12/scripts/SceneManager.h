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
	//シーンの描画
	void drawScene3D();
	void drawScene2D();
	void drawScene();

	//シーン中のアクター配列を更新
	void joinSceneActors();

	//シーン中のアクターの削除
	void removeSceneActors();

	//getter
	const std::string& getCurrentSceneType();
	class Scene& getCurrentScene();

	void transitToTitle();
	void transitToTown();
	void transitToMap();
	void transitToGameOver();
	void transitScene();

#ifdef _DEBUG
	void drawDebugGUI();
#endif
private:
	std::string mNextSceneType;
	std::string mCurrentSceneType;
	class Scene* mCurrentScene;
	std::unordered_map<std::string, std::unique_ptr<Scene>> mSceneMap;

	Game& mGame;
};

