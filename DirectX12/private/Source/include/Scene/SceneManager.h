#pragma once
#include <string>
#include <unordered_map>

class Game;

class SceneManager
{
public:
	SceneManager(Game& game);
	~SceneManager() = default;

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
	class Scene& getCurrentScene();

	//シーンの追加
	void addScene(const std::string& sceneName, std::unique_ptr<class Scene>& scene);

	//シーン遷移
	void setNextScene(const std::string& nextSceneType);
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

