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
	mAssetManager = std::make_unique<AssetManager>(*mGraphic.get());

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

	mGraphic->end3DRender();

	mGraphic->moveToNextFrame();


}
