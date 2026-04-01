#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Actor.h"

class SceneManager;
class MeshComponent;
class SpriteComponent;
class TextComponent;
class PointLightComponent;
class SpotLightComponent;

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

	//描画処理
	void draw3D();
	void draw2D();
	virtual	void drawScene() {};

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

	//メッシュの追加/削除
	void addMesh(MeshComponent* mesh);
	void removeMesh(MeshComponent* mesh);
	//スプライトの追加/削除
	void addSprite(SpriteComponent* sprite);
	void removeSprite(SpriteComponent* sprite);
	//テキストの追加/削除
	void addText(TextComponent* text);
	void removeText(TextComponent* text);
	//点光源の追加/削除
	void addPointLight(PointLightComponent* light);
	void removePointLight(PointLightComponent* light);
	std::vector<PointLightComponent*>& getPointLights() { return mPointLights; }
	//スポットライトの追加/削除
	void addSpotLight(SpotLightComponent* light);
	void removeSpotLight(SpotLightComponent* light);
	std::vector<SpotLightComponent*>& getSpotLights() { return mSpotLights; }

	//アクターの全消去
	void refreshActors();
protected:
	Game& mGame;

private:
	std::vector<std::unique_ptr<Actor>> mActors;
	std::vector<std::unique_ptr<Actor>> mPendingActors;
	std::vector<MeshComponent*> mMeshes;
	std::vector<SpriteComponent*> mSprites;
	std::vector<TextComponent*> mTexts;
	std::vector<PointLightComponent*> mPointLights;
	std::vector<SpotLightComponent*> mSpotLights;
};

