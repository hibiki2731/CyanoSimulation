#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <DirectXMath.h>
#include <cassert>
#include <random>
#include "BIN_FILE12.h"
#include "Definition.h"
#include "FBXConverter.h"
#include "timer.h"
#include <wrl/client.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

//前方宣言
class Graphic;
class Actor;
class Component;
class MeshComponent;
class SpriteComponent;
class PointLightComponent;
class SpotLightComponent;
class Player;
class TextComponent;
class EnemyComponent;
class DamageTextManager;
class AssetManager;
class SceneManager;
class ItemManager;
class PlayerManager;
class AudioManager;

class Game {
public:
	Game();
	~Game();
	//ループの開始
	void runLoop();
	//終了処理
	int endProcess();
	//初期化
	void init();

	//点光源の追加
	void addPointLight(PointLightComponent* light);
	void removePointLight(PointLightComponent* light);
	//スポットライトの追加
	void addSpotLight(SpotLightComponent* light);
	void removeSpotLight(SpotLightComponent* light);

	//ゲッター
	Graphic* getGraphic();
	AssetManager* getAssetManager();
	std::vector<PointLightComponent*>& getPointLights();
	std::vector<SpotLightComponent*>& getSpotLights();
	ItemManager* getItemManager();
	SceneManager& getSceneManager();
	PlayerManager* getPlayerManager();
	AudioManager* getAudioManager();

private:
	bool mUpdatingActors;

	//グラフィック
	std::unique_ptr<Graphic> mGraphic;
	//ライト配列
	std::vector<PointLightComponent*> mPointLights;
	std::vector<SpotLightComponent*> mSpotLights;

	//プレイヤー
	std::unique_ptr<PlayerManager> mPlayerManager;

	//AssetManager
	std::unique_ptr<AssetManager> mAssetManager;

	//ItemManger
	std::unique_ptr<ItemManager> mItemManager;

	//シーンマネージャー
	std::unique_ptr<SceneManager> mSceneManager;

	//オーディオマネージャー
	std::unique_ptr<AudioManager> mAudioManager;

	//ループ内処理
	void input();
	void update();
	void draw();
};
