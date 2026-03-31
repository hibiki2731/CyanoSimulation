#include "Game.h"
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
#include "EnemyComponent.h"
#include "DamageText.h"
#include "Graphic.h"
#include "input.h"
#include "ItemManager.h"
#include "json.hpp"
#include "PlayerManager.h"
#include "AudioManager.h"
#include "Scene.h"

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
	mGraphic = std::make_unique<Graphic>(*this);
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


	//assetManagerの初期化 meshComponentを作成する前に初期化
	mAssetManager = std::make_unique<AssetManager>(mGraphic.get());

	//itemManagerの初期化
	mItemManager = std::make_unique<ItemManager>();

	//damageTextの初期化
	mDamageTextManager = std::make_unique<DamageTextManager>(*this);

	//PlayerManager
	mPlayerManager = std::make_unique<PlayerManager>();

	//AudioManager
	mAudioManager = std::make_unique<AudioManager>();

	//シーンマネージャーの初期化	
	mSceneManager = std::make_unique<SceneManager>(*this);

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

Graphic* Game::getGraphic()
{
	return mGraphic.get();
}

DamageTextManager* Game::getDamageTextManager()
{
	return mDamageTextManager.get();
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

SceneManager& Game::getSceneManager()
{
	return *mSceneManager.get();
}

PlayerManager* Game::getPlayerManager()
{
	return mPlayerManager.get();
}

AudioManager* Game::getAudioManager()
{
	return mAudioManager.get();
}

void Game::input()
{
	updateInput();
	//アクターの入力処理
	mSceneManager->inputScene();


#ifdef _DEBUG
	//デバック用	}
	if (GetAsyncKeyState('T')) {
		mSceneManager->transitToTitle();
	}
	if (GetAsyncKeyState('H')) {
		mSceneManager->transitToTown();
	}

	if (GetAsyncKeyState('P')) {
		mAudioManager->playBGM("UI_MOVE");
	}
#endif

}

void Game::update()
{
	mUpdatingActors = true;

	//早めのシーン更新
	mSceneManager->fastUpdateScene();

	//基本のシーン更新
	mSceneManager->updateScene();


	//各種マネージャーの更新
	{
		mSceneManager->transitScene();	//シーンの移行
		mDamageTextManager->update();	//ダメージテキストの更新
	}

	mSceneManager->joinSceneActors();	//シーンにアクターを追加

	//死んだアクターの除去
	mSceneManager->removeSceneActors();

	//アクターの除去後に行う処理
	{
		mSceneManager->lateUpdateScene();
		//各種マネージャーの更新
		mGraphic->updateBase3DData();		//Base3DDataの更新
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
