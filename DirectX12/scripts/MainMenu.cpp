#include "MainMenu.h"
#include "Game.h"
#include "AudioManager.h"
#include "InnMenu.h"
#include "ShopMenu.h"
#include "ForgeMenu.h"
#include "ExplorerMenu.h"
#include "SceneManager.h"

//各種メニューのコンストラクタ
MainMenu::MainMenu(Game& game,TownManager& townManager, float zDepth) : Menu(game, townManager, "MainMenu", zDepth)
{
	mMaxIndex = 5;
}

//各種メニューのupdate
void MainMenu::selectedAct() {
	mGame.getAudioManager()->playSE("UI_WINDOW_OPEN");
	switch (mSelectedIndex) {
	case 0: {
		auto inn = std::make_unique<InnMenu>(mGame, mTownManager, 97.0f);
		mGame.addActor(std::move(inn));
		break;
	}
	case 1: {
		auto shop = std::make_unique<ShopMenu>(mGame, mTownManager, 97.0f);
		mGame.addActor(std::move(shop));
		break;
	}
	case 2: {
		auto forge = std::make_unique<ForgeMenu>(mGame, mTownManager, 97.0f);
		mGame.addActor(std::move(forge));
		break;
	}
	case 3: {
		auto explorer = std::make_unique<ExplorerMenu>(mGame, mTownManager, 97.0f);
		mGame.addActor(std::move(explorer));
		break;
	}
	case 4: {
		mGame.getSceneManager()->transitToMap();
	}
	}
}

