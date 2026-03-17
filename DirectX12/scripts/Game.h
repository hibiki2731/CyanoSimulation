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
class MapManager;
class EnemyComponent;
class DamageTextManager;
class AssetManager;
class SceneManager;
class TownManager;
class ItemManager;
class PlayerManager;


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

	//アクターの追加
	void addActor(std::unique_ptr<Actor> actor);
	//コンポーネント配列への追加、除去
	//メッシュの追加
	void addMesh(MeshComponent* mesh);
	void removeMesh(MeshComponent* mesh);
	//スプライトの追加
	void addSprite(SpriteComponent* mesh);
	void removeSprite(SpriteComponent* mesh);
	//点光源の追加
	void addPointLight(PointLightComponent* light);
	void removePointLight(PointLightComponent* light);
	//スポットライトの追加
	void addSpotLight(SpotLightComponent* light);
	void removeSpotLight(SpotLightComponent* light);
	//テキストの追加
	void addText(TextComponent* text);
	void removeText(TextComponent* text);
	//エネミーの追加
	void addEnemy(EnemyComponent* enemy);
	void removeEnemy(EnemyComponent* enemy);
	void activateEnemies();

	void clearActors();
	//ゲッター
	Graphic* getGraphic();
	std::vector<EnemyComponent*>& getEnemies();
	MapManager* getMapManager();
	DamageTextManager* getDamageTextManager();
	EnemyComponent* getEnemyFromIndexPos(const int x, const int y);
	EnemyComponent* getEnemyFromIndexPos(const int index);
	AssetManager* getAssetManager();
	std::vector<PointLightComponent*>& getPointLights();
	std::vector<SpotLightComponent*>& getSpotLights();
	ItemManager* getItemManager();
	SceneManager* getSceneManager();
	TownManager* getTownManager();
	PlayerManager* getPlayerManager();

private:

	//グラフィック
	std::unique_ptr<Graphic> mGraphic;
	//アクター配列（所有権を持つ）
	std::vector<std::unique_ptr<Actor>> mActors;
	std::vector<std::unique_ptr<Actor>> mPendingActors;
	//コンポーネント配列（所有権は持たない）
	//メッシュ配列
	std::vector<MeshComponent*> mMeshes;
	std::vector<SpriteComponent*> mSprites;
	//テキスト配列
	std::vector<TextComponent*> mTexts;
	//ライト配列
	std::vector<PointLightComponent*> mPointLights;
	std::vector<SpotLightComponent*> mSpotLights;
	//キャラクター配列
	std::vector<EnemyComponent*> mEnemies;
	//ダメージエフェクト用
	std::unique_ptr<DamageTextManager> mDamageTextManager;

	//マップ関連
	std::unique_ptr<MapManager> mMapManager;
	bool mUpdatingActors;

	//タウンマネージャー
	std::unique_ptr<TownManager> mTownManager;

	//プレイヤー
	std::unique_ptr<PlayerManager> mPlayerManager;

	//AssetManager
	std::unique_ptr<AssetManager> mAssetManager;

	//ItemManger
	std::unique_ptr<ItemManager> mItemManager;

	//シーンマネージャー
	std::unique_ptr<SceneManager> mSceneManager;

	//ループ内処理
	void input();
	void update();
	void draw();
};
