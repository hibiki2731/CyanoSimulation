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
class DamageTextGenerator;
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

	//ゲッター
	Graphic& getGraphic();
	AssetManager& getAssetManager();
	ItemManager& getItemManager();
	SceneManager& getSceneManager();
	PlayerManager& getPlayerManager();
	AudioManager& getAudioManager();

private:
	bool mUpdatingActors;

	//グラフィック
	std::unique_ptr<Graphic> mGraphic;

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
