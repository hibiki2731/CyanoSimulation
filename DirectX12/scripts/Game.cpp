#include "Game.h"
#include "TownManager.h"
#include "SceneManager.h"
#include "Actor.h"
#include "Player.h"
#include "UI.h"
#include "Player.h"
#include "MessageWindow.h"
#include "MeshComponent.h"
#include "SpriteComponent.h"
#include "PointLightComponent.h"
#include "PointLight.h"
#include "TextComponent.h"
#include "Enemy.h"
#include "MapManager.h"
#include "EnemyComponent.h"
#include "DamageText.h"
#include "Graphic.h"
#include "input.h"
#include "ItemManager.h"
#include "json.hpp"
#include "PlayerManager.h"

Game::Game(){
	mUpdatingActors = false;
}

Game::~Game()
{
}

void Game::runLoop()
{
	while (!mGraphic->quit()) {
		setDeltaTime();

		input();
		update();
		draw();
	}
}

int Game::endProcess()
{
	mGraphic->waitGPU();
	mGraphic->closeEventHandle();
	return mGraphic->msg_wparam();
}

void Game::init() {
	mGraphic = std::make_unique<Graphic>(this);
	mGraphic->init();
	mGraphic->clearColor(0.25f, 0.5f, 0.9f);

	//FBXファイルとテキストファイルのパス
	const char* fbx[] = { 
		"assets/3DModels/rockObj/rockWall.fbx",
		"assets/3DModels/rockObj/rockFloor.fbx",
		"assets/3DModels/Grass/grass.fbx",
		"assets/3DModels/Slime/slime.fbx",
		"assets/3DModels/Nurikabe/nurikabe.fbx"
	};

	const char* text[] = { 
		"assets/3DModels/rockObj/rockWall.txt",
		"assets/3DModels/rockObj/rockFloor.txt",
		"assets/3DModels/Grass/grass.txt",
		"assets/3DModels/Slime/slime.txt",
		"assets/3DModels/Nurikabe/nurikabe.txt"
	};

#ifdef _DEBUG
	//FBX→テキスト変換
	FBXConverter fbxConverter;
	const int fbxNum = _countof(fbx);
	int i;
	//フリー素材
	for (i = 0; i < fbxNum; i++) {
		fbxConverter.fbxToTxt(fbx[i], text[i], 1.0f, 1.0f, 1.0f, 0, 1, 2); //横、縦、奥行
		
	}
#endif


	//タイマー初期化
	initDeltaTime();

	//シーンマネージャーの初期化	
	mSceneManager = std::make_unique<SceneManager>(this);

	//assetManagerの初期化 meshComponentを作成する前に初期化
	mAssetManager = std::make_unique<AssetManager>(mGraphic.get());

	//mapの生成
	mMapManager = std::make_unique<MapManager>(this);
	mMapManager->setStage(Stage::MAP1);
	
	//TownManagerの初期化
	mTownManager = std::make_unique<TownManager>(this);

	//itemManagerの初期化
	mItemManager = std::make_unique<ItemManager>();

	//damageTextの初期化
	mDamageTextManager = std::make_unique<DamageTextManager>(this);

	//PlayerManager
	mPlayerManager = std::make_unique<PlayerManager>();

}

void Game::addActor(std::unique_ptr<Actor> actor)
{
	//アクターの更新中なら待ちに追加
	if (mUpdatingActors) {
		mPendingActors.emplace_back(std::move(actor));
	}
	else {
		mActors.emplace_back(std::move(actor));
	}
}

void Game::addMesh(MeshComponent* mesh)
{
	mMeshes.emplace_back(mesh);
}

void Game::removeMesh(MeshComponent* mesh)
{
	auto iter = std::find(mMeshes.begin(), mMeshes.end(), mesh);
	if (iter != mMeshes.end()) {
		std::iter_swap(iter, mMeshes.end() - 1);
		mMeshes.pop_back();
	}
}
void Game::addSprite(SpriteComponent* sprite)
{
	mSprites.emplace_back(sprite);
}

void Game::removeSprite(SpriteComponent* sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	if (iter != mSprites.end()) {
		std::iter_swap(iter, mSprites.end() - 1);
		mSprites.pop_back();
	}
}

void Game::addPointLight(PointLightComponent* light)
{
	if (mPointLights.size() > MAX_LIGHT_NUM) return;
	mPointLights.emplace_back(light);
}

void Game::removePointLight(PointLightComponent* light)
{
	mPointLights.erase(std::remove(mPointLights.begin(), mPointLights.end(), light), mPointLights.end());
}

void Game::addSpotLight(SpotLightComponent* light)
{
	if (mSpotLights.size() > MAX_LIGHT_NUM) return;
	mSpotLights.emplace_back(light);
}

void Game::removeSpotLight(SpotLightComponent* light)
{
	mSpotLights.erase(std::remove(mSpotLights.begin(), mSpotLights.end(), light), mSpotLights.end());
}

void Game::addText(TextComponent* text)
{
	mTexts.emplace_back(text);
}

void Game::removeText(TextComponent* fontText)
{
	mTexts.erase(std::remove(mTexts.begin(), mTexts.end(), fontText), mTexts.end());
}

void Game::addEnemy(EnemyComponent* enemy)
{
	mEnemies.emplace_back(enemy);
}

void Game::removeEnemy(EnemyComponent* enemy)
{
	auto iter = std::find(mEnemies.begin(), mEnemies.end(), enemy);
	if (iter != mEnemies.end()) {
		std::iter_swap(iter, mEnemies.end() - 1);
		mEnemies.pop_back();
	}
}

void Game::activateEnemies()
{
	for (auto enemy : mEnemies) {
		enemy->activate();
	}
}

void Game::clearActors()
{
	for (auto& actor : mActors) {
		actor->setState(Actor::State::Dead);
	}
}

Graphic* Game::getGraphic()
{
	return mGraphic.get();
}

std::vector<EnemyComponent*>& Game::getEnemies()
{
	return mEnemies;
}

MapManager* Game::getMapManager()
{
	return mMapManager.get();
}

DamageTextManager* Game::getDamageTextManager()
{
	return mDamageTextManager.get();
}

EnemyComponent* Game::getEnemyFromIndexPos(int x, int y)
{
	for (auto enemy : mEnemies) {
		std::vector<int> charIndexPos = enemy->getIndexPos();
		if (charIndexPos[0] == x && charIndexPos[1] == y) {
			return enemy;
		}
	}
	return nullptr;
}

EnemyComponent* Game::getEnemyFromIndexPos(int index)
{
	int mapSize = mMapManager->getMapSize();
	int x = index % mapSize;
	int y = index / mapSize;
	for (auto enemy : mEnemies) {
		std::vector<int> charIndexPos = enemy->getIndexPos();
		if (charIndexPos[0] == x && charIndexPos[1] == y) {
			return enemy;
		}
	}

	return nullptr;
}

AssetManager* Game::getAssetManager()
{
	return mAssetManager.get();
}

std::vector<PointLightComponent*>& Game::getPointLights()
{
	return mPointLights;
}

std::vector<SpotLightComponent*>& Game::getSpotLights()
{
	return mSpotLights;
}

ItemManager* Game::getItemManager()
{
	return mItemManager.get();
}

SceneManager* Game::getSceneManager()
{
	return mSceneManager.get();
}

TownManager* Game::getTownManager()
{
	return mTownManager.get();
}

PlayerManager* Game::getPlayerManager()
{
	return mPlayerManager.get();
}

void Game::input()
{
	updateInput();

	for (auto& actor : mActors) {

		actor->input();
	}

	//各種マネージャーの入力
	mTownManager->input();

#ifdef _DEBUG
	//デバック用
	if (GetAsyncKeyState('P')) {
		auto slime = std::make_unique<Enemy>(this, CharacterType::SLIME, static_cast<float>(MAPTIPSIZE * 5.0f), static_cast<float>(MAPTIPSIZE * 5.0f));
		addActor(std::move(slime));
	}
	if (GetAsyncKeyState('O')) {
		mMapManager->moveToPlayerTurn();
	}
	if (GetAsyncKeyState('T')) {
		mSceneManager->transitToTitle();
	}
	if (GetAsyncKeyState('H')) {
		mSceneManager->transitToTown();
	}
#endif

}

void Game::update()
{
	mUpdatingActors = true;
	//アクターの更新処理
	{
		for (auto& actor : mActors) {
			actor->update();
		}
		//敵の更新処理
		for (auto enemy : mEnemies) {
			enemy->updateActiveProcess();
		}
	}

	//各種マネージャーの更新
	{
		mSceneManager->transitScene();	//シーンの移行
		mDamageTextManager->update();	//ダメージテキストの更新
		mTownManager->update();			//シーンがタウンの時の処理
		mMapManager->sceneProcess(); //シーン変更時の初期化、終了処理
	}

	//Actorの更新中に追加されたアクターをActor配列に追加
	mUpdatingActors = false;
	{
		for (auto& pending : mPendingActors) {
			mActors.emplace_back(std::move(pending));
		}
		mPendingActors.clear();
	}

	//死んだアクターの除去
	{
		//死んだアクターを一次配列に追加
		std::vector<std::unique_ptr<Actor>> deadActors;
		for (auto& actor : mActors) {
			if (actor->getState() == Actor::Dead) {
				actor->endProccess();
				deadActors.emplace_back(std::move(actor));
			}
		}

		//元配列に残ったnullptrを削除
		std::erase_if(mActors, [](const std::unique_ptr<Actor>& actor) {
			return actor == nullptr;
			});



	}

	//アクターの除去後に行う処理
	{
		//敵配列をプレイヤーに近い順にソート
		std::sort(mEnemies.begin(), mEnemies.end(), [](auto const lenemy, auto const renemy) {
			return lenemy->getDist() < renemy->getDist();
			});

		//各種マネージャーの更新
		mGraphic->updateBase3DData();		//Base3DDataの更新
		mMapManager->updateTurn();				//ターン制御
	}
}

void Game::draw()
{

	//3D描画
	mGraphic->begin3DRender();
	mGraphic->setRenderType(Graphic::RENDER_3D);
	for (auto& mesh : mMeshes) {
		mesh->draw();
	}

	//2D描画
	mGraphic->setRenderType(Graphic::RENDER_2D);
	for (auto& sprite : mSprites) {
		sprite->draw();
	}
	for (auto& text : mTexts) {
		text->draw();
	}

	//ダメージエフェクト
	mGraphic->setRenderType(Graphic::RENDER_DT);
	mDamageTextManager->draw();

	mGraphic->end3DRender();

	mGraphic->moveToNextFrame();


}
