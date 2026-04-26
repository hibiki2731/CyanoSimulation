#include "Game.h"
#include "SceneManager.h"
#include "Actor.h"
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
#include "GUIDebugger.h"

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

#ifdef _DEBUG
	//GUIControllerの初期化
	mGUIDebugger = std::make_unique<GUIDebugger>(*mGraphic.get());
#endif

	//タイマー初期化
	initDeltaTime();

	//assetManagerの初期化 meshComponentを作成する前に初期化
	mAssetManager = std::make_unique<AssetManager>(*mGraphic.get());

	//ビルボード処理用のバッファを初期化
	mGraphic->initBilbordBuffer();

	//itemManagerの初期化
	mItemManager = std::make_unique<ItemManager>();

	//PlayerManager
	mPlayerManager = std::make_unique<PlayerManager>(*this);

	//AudioManager
	mAudioManager = std::make_unique<AudioManager>();

	//シーンマネージャーの初期化	
	mSceneManager = std::make_unique<SceneManager>(*this);


}

Graphic& Game::getGraphic()
{
	return *mGraphic.get();
}

AssetManager& Game::getAssetManager()
{
	return *mAssetManager.get();
}

ItemManager& Game::getItemManager()
{
	return *mItemManager.get();
}

SceneManager& Game::getSceneManager()
{
	return *mSceneManager.get();
}

PlayerManager& Game::getPlayerManager()
{
	return *mPlayerManager.get();
}

AudioManager& Game::getAudioManager()
{
	return *mAudioManager.get();
}


void Game::input()
{
	updateInput();

	//フェード中は入力を受け付けない
	if (mGraphic->isFading()) return;

	//アクターの入力処理
	mSceneManager->inputScene();
}

void Game::update()
{
	mUpdatingActors = true;

	//シーンの移行
	mSceneManager->transitScene();	//シーンの移行

	//早いシーン更新
	mSceneManager->fastUpdateScene();

	//基本のシーン更新
	mSceneManager->updateScene();
 
	//シーンにアクターを追加
	mSceneManager->joinSceneActors();

	//死んだアクターの除去
	mSceneManager->removeSceneActors();

	//遅いシーン更新
	mSceneManager->lateUpdateScene();

	//フェードの更新
	mGraphic->updateFade();
}

void Game::draw()
{

	//3D描画
	mGraphic->begin3DRender();


	mGraphic->setRenderType(Graphic::RENDER_3D);
	mSceneManager->drawScene3D();

	//2D描画
	mGraphic->setRenderType(Graphic::RENDER_2D);
	mSceneManager->drawScene2D();

	//シーン独自の描画
	mSceneManager->drawScene();

	//フェード処理
	mGraphic->renderFade();

#ifdef _DEBUG
	mGUIDebugger->begin();
	mSceneManager->drawDebugGUI();
	mGUIDebugger->end();	
#endif


	mGraphic->end3DRender();

	mGraphic->moveToNextFrame();


}

#ifdef _DEBUG

GUIDebugger& Game::getGUIDebugger()
{
	return *mGUIDebugger.get();
}
#endif