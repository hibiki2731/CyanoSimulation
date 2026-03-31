#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Actor.h"

class SceneManager;

class Scene
{
public:
	Scene(class Game& game);
	virtual ~Scene() {};

	//更新処理
	void fastUpdate();
	void update();
	void lateUpdate();
	//シーン中のアクターの更新
	void fastUpdateActors();
	void updateActors();
	void lateUpdateActors();
	//シーン独自の更新処理
	virtual void fastUpdateScene() {};
	virtual void updateScene() {};
	virtual void lateUpdateScene() {};

	//入力処理
	void input();
	//アクターの入力処理
	void inputActors();
	//シーン独自の入力処理
	virtual void inputScene() {};

	virtual void onEnter() = 0;
	virtual void onExit() = 0;

	//シーン名の取得
	virtual const std::string& getName() const = 0;

	//Gameの取得
	Game& getGame() { return mGame; }

	//アクターの追加/削除
	void addActor(std::unique_ptr<Actor> actor);
	void joinActors();
	void removeActors();

	//アクターの全消去
	void refreshActors();
protected:
	Game& mGame;

private:
	std::vector<std::unique_ptr<Actor>> mActors;
	std::vector<std::unique_ptr<Actor>> mPendingActors;

};

