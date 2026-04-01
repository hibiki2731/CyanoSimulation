#include "MainMenu.h"
#include "Game.h"
#include "TownScene.h"
#include "AudioManager.h"
#include "InnMenu.h"
#include "ShopMenu.h"
#include "ForgeMenu.h"
#include "ExplorerMenu.h"
#include "SceneManager.h"

//各種メニューのコンストラクタ
MainMenu::MainMenu(TownScene& scene, float zDepth) : Menu(scene, "MainMenu", zDepth)
{
	mMaxIndex = 5;
}

//各種メニューのupdate
void MainMenu::selectedAct() {
	mScene.getGame().getAudioManager()->playSE("UI_WINDOW_OPEN");
	switch (mSelectedIndex) {
	case 0: {
		auto inn = std::make_unique<InnMenu>(mScene, 97.0f);
		mScene.addActor(std::move(inn));
		break;
	}
	case 1: {
		auto shop = std::make_unique<ShopMenu>(mScene, 97.0f);
		mScene.addActor(std::move(shop));
		break;
	}
	case 2: {
		auto forge = std::make_unique<ForgeMenu>(mScene, 97.0f);
		mScene.addActor(std::move(forge));
		break;
	}
	case 3: {
		auto explorer = std::make_unique<ExplorerMenu>(mScene, 97.0f);
		mScene.addActor(std::move(explorer));
		break;
	}
	case 4: {
		mScene.getGame().getSceneManager().transitToMap();
	}
	}
}

