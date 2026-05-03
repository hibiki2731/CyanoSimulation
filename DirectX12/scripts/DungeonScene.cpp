#include "Definition.h"
#include "DungeonScene.h"
#include "MapGenerator.h"
#include "TurnObserver.h"
#include "Game.h"
#include "SceneManager.h"
#include "Enemy.h"
#include "Player.h"
#include "Resource.h"
#include "EnemyComponent.h"
#include "MessageWindow.h"
#include "DamageText.h"
#include "MiniMap.h"
#include "Graphic.h"
#include "DungeonUI.h"
#include "AudioManager.h"
#include "FireParticleComponent.h"
#include "DebugCamera.h"

DungeonScene::DungeonScene(Game& game)
	:Scene(game)
{
	mMapGenerator = std::make_unique<MapGenerator>(*this);
	mTurnObserver = std::make_unique<TurnObserver>(*this);
	mDamageTextManaager = std::make_unique<DamageTextGenerator>(game);
	mPlayer = nullptr;
	mMapSize = 0;
	mMaxResourcePoint = 5;
	mStage = Stage::MAP1;
}

void DungeonScene::fastUpdateScene() {
	mTurnObserver->updateTurn();
}

void DungeonScene::updateScene()
{
	mDamageTextManaager->update();
}

void DungeonScene::lateUpdateScene()
{
	//光源の更新
	mGame.getGraphic().updateBase3DData(getPointLights(), getSpotLights());
}

void DungeonScene::drawScene()
{
	//ダメージテキストの描画
	mGame.getGraphic().setRenderType(Graphic::RENDER_DT);
	mDamageTextManaager->draw();

	//炎パーティクルの描画
	mGame.getGraphic().setRenderType(Graphic::RENDER_FP);
	for (auto p : mParticles) p->draw();

}

void DungeonScene::onEnter()
{
	mMapGenerator->begin();
	mTurnObserver->begin();

	mGame.getGraphic().startFadeIn(1.0f); 
	mGame.getAudioManager().playBGM("BGM_DUNGEON");

#ifdef _DEBUG
	mDebugFlag = true;

#ifdef EDIT
	//プレイヤーを削除
	mPlayer->setState(Actor::State::Dead);
	mPlayer = nullptr;

	//デバッグ用カメラの生成
	auto camera = std::make_unique<DebugCamera>(*this);
	addActor(std::move(camera));

	return;
#endif

#endif

	//ミニマップの作成
	auto minimap = std::make_unique<MiniMap>(*this);
	mMiniMap = minimap.get();
	addActor(std::move(minimap));
	mMiniMap->updatePosition();

	//UIの作成
	auto dungeonUI = std::make_unique<DungeonUI>(*this);
	mUI = dungeonUI.get();
	addActor(std::move(dungeonUI));


}

void DungeonScene::onExit()
{
	mMapGenerator->end();
	mResources.clear();

#ifdef _DEBUG
	mDebugFlag = false;
#endif
}

void DungeonScene::createEnemy(const std::string& enemyID, float x, float y)
{
	//敵の生成
	std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(*this, enemyID, x, y);
	addActor(std::move(enemy)); //所有権をGameへ渡す
}

void DungeonScene::createPlayer(float x, float y)
{
	//プレイヤー生成
	std::unique_ptr player = std::make_unique<Player>(*this, x, y);
	mPlayer = player.get();
	addActor(std::move(player)); //所有権をSceneへ渡す
}


void DungeonScene::createResource(const std::string& resourceID, const std::string& meshID, float x, float y, int index)
{
	//草の生成
	std::unique_ptr<Resource> resource = std::make_unique<Resource>(*this, resourceID, meshID, x, y, index);
	mResources[index] = resource.get();
	addActor(std::move(resource)); //所有権をGameへ渡す
}

void DungeonScene::spawnResource()
{
	if (mResources.size() >= mMaxResourcePoint) return;

	int playerIndex[2];
	mPlayer->getIndexPos(playerIndex);

	int i = 0;
	//障害物がない　かつ　プレイヤーから4マス以上9マス以下の範囲でスポーン
	while (i < 50) {
		//スポーンするマスを乱数で決定
		int x = playerIndex[0] + Random::dist(-9, 9);
		int y = playerIndex[1] + Random::dist(-9, 9);

		//障害物がある場合、もう一度乱数を振りなおす
		if (x < 0 || x >= mMapSize || y < 0 || y >= mMapSize) {
			i++;
			continue;
		}
		if (mTileData[x][y] != TileType::FLOOR
			|| mCharacterData[x][y] != CharacterType::EMPTY) {
			i++;
			continue;
		}
		
		//プレイヤーから2マスいないならば、もう一度乱数を振りなおす
		int distance = abs(playerIndex[0] - x) + abs(playerIndex[1] - y);
		if (distance <= 2) {
			i++;
			continue;
		}

		//リソースの生成
		std::string resourceID;
		std::string meshID;

		switch (mStage) {
		case Stage::MAP1:
			resourceID = "GRASS";
			meshID = "GRASS";
			break;
		default:
			resourceID = "GRASS";
			meshID = "GRASS";
			break;
		}

		createResource(resourceID, meshID, static_cast<float>(MAPTIPSIZE * x), static_cast<float>(MAPTIPSIZE * y), y * mMapSize + x);
		break;
	}
}

void DungeonScene::deleteResourceFromIndex(int index)
{
	mResources.erase(index);
}

void DungeonScene::returnToTown()
{
	mGame.getSceneManager().transitToTown();
}

void DungeonScene::transitToGameOver()
{
	mGame.getSceneManager().transitToGameOver();
}

void DungeonScene::setTileDataAt(int x, int y, int data)
{
	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mTileData[x][y] = data;
}

void DungeonScene::setTileDataAt(int index, int data)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mTileData[x][y] = data;
}

void DungeonScene::setTileData(const std::vector<std::vector<int>>& mapData)
{
	mTileData = mapData;
}

void DungeonScene::setCharacterDataAt(int x, int y, int data)
{
	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mCharacterData[x][y] = data;
}

void DungeonScene::setCharacterDataAt(int index, int data)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら無効
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return;

	mCharacterData[x][y] = data;
}

void DungeonScene::setCharacterData(const std::vector<std::vector<int>>& objectData)
{
	mCharacterData = objectData;
}

int DungeonScene::getTileDataAt(int x, int y)
{
	//配列範囲外なら壁を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return TileType::WALL;

	return mTileData[x][y];
}

int DungeonScene::getTileDataAt(int index)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら壁を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return TileType::WALL;
	return mTileData[x][y];
}

int DungeonScene::getCharacterDataAt(int x, int y)
{
	//配列範囲外なら空を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return CharacterType::EMPTY;

	return mCharacterData[x][y];
}

int DungeonScene::getCharacterDataAt(int index)
{
	int x = index % mMapSize;
	int y = index / mMapSize;

	//配列範囲外なら空を出力
	if (x < 0 || x > mMapSize - 1 ||
		y < 0 || y > mMapSize - 1) return CharacterType::EMPTY;

	return mCharacterData[x][y];
}
const Stage& DungeonScene::getStage()
{
	return mStage;
}
void DungeonScene::addEnemy(EnemyComponent* enemy)
{
	mEnemies.emplace_back(enemy);
}

void DungeonScene::removeEnemy(EnemyComponent* enemy)
{
	mEnemies.erase(std::remove(mEnemies.begin(), mEnemies.end(), enemy), mEnemies.end());
}

void DungeonScene::sortEnemiesByDistanceToPlayer()
{
	//敵配列をプレイヤーに近い順にソート
	std::sort(mEnemies.begin(), mEnemies.end(), [](auto const lenemy, auto const renemy) {
		return lenemy->getDist() < renemy->getDist();
		});
}

void DungeonScene::addParticle(FireParticleComponent* particle)
{
	mParticles.push_back(particle);
}

void DungeonScene::removeParticle(FireParticleComponent* particle)
{
	mParticles.erase(std::remove(mParticles.begin(), mParticles.end(), particle), mParticles.end());
}

void DungeonScene::updateMiniMapPos()
{
	mMiniMap->updatePosition();
}

void DungeonScene::updateMiniMapDirection()
{
	mMiniMap->updateDirection();
}

void DungeonScene::updateHPUI()
{
	mUI->updateHP();
}

void DungeonScene::updateAPUI()
{
	mUI->updateAP();
}

void DungeonScene::updateItemUI()
{
	mUI->updateItemIcon();
}

void DungeonScene::updateItemFrame()
{
	mUI->updateItemFrame();
}

void DungeonScene::pushMessage(const std::string& message)
{
	mUI->pushMessage(message);
}

void DungeonScene::updateGold()
{
	mUI->updateGold();
}

void DungeonScene::moveToEnemyTurn()
{
	mTurnObserver->moveToEnemyTurn();
}

void DungeonScene::moveNextFloor()
{

}

void DungeonScene::moveToPlayerTurn()
{
	mTurnObserver->moveToPlayerTurn();
}

EnemyComponent* DungeonScene::getEnemyFromIndexPos(int index)
{
	int x = index % mMapSize;
	int y = index / mMapSize;
	for (auto enemy : mEnemies) {
		std::vector<int> charIndexPos = enemy->getIndexPos();
		if (charIndexPos[0] == x && charIndexPos[1] == y) {
			return enemy;
		}
	}
	return nullptr;
}

EnemyComponent* DungeonScene::getEnemyFromIndexPos(int x, int y)
{
	for (auto enemy : mEnemies) {
		std::vector<int> charIndexPos = enemy->getIndexPos();
		if (charIndexPos[0] == x && charIndexPos[1] == y) {
			return enemy;
		}
	}
	return nullptr;
}

int DungeonScene::getPlayerActLimit()
{
	return (mPlayer) ? mPlayer->getAP() : 0;
}

class Resource* DungeonScene::getResource(int index)
{
	auto iter = mResources.find(index);
	if (iter != mResources.end()) {
		return iter->second;
	}
	else {
		return nullptr;
	}
}

class Resource* DungeonScene::getResource(int x, int y)
{
	int index = y * mMapSize + x;
	return getResource(index);
}

TurnType DungeonScene::getTurnType() const
{
	return mTurnObserver->getTurnType();
}

float DungeonScene::getDamageTextNum() const
{
	return mDamageTextManaager->getSize();
}

void DungeonScene::createDamageText(const XMFLOAT3& pos, int digit)
{
	mDamageTextManaager->createDamageText(pos, digit);
}

const TurnObserver& DungeonScene::getTurnObserver()
{
	return *mTurnObserver.get();
}

