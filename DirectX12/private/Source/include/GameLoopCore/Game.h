#pragma once
#include <cassert>
#include <random>
#include "Utility/FileConverter/FBXConverter.h"
#include "Graphic/Core/Definition.h"
#include "Utility/Time/timer.h"

using namespace DirectX;

//前方宣言
class Graphic;
class Actor;
class Component;
class SpriteComponent;
class TextComponent;
class AssetManager;
class SceneManager;
class AudioManager;
class GUIDebugger;

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
	SceneManager& getSceneManager();
	AudioManager& getAudioManager();

#ifdef _DEBUG
	GUIDebugger& getGUIDebugger();
#endif

	class EngineResourceFactory createFactory();

private:
	bool mUpdatingActors;

	//グラフィック
	std::unique_ptr<Graphic> mGraphic;

	//AssetManager
	std::unique_ptr<AssetManager> mAssetManager;

	//シーンマネージャー
	std::unique_ptr<SceneManager> mSceneManager;

	//オーディオマネージャー
	std::unique_ptr<AudioManager> mAudioManager;

#ifdef _DEBUG
	//GUI編集用
	std::unique_ptr<GUIDebugger> mGUIDebugger;
#endif 

	//ループ内処理
	void input();
	void update();
	void draw();

};
