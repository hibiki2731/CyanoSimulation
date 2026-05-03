#pragma once
#include "Scene.h"
#include <memory>
#include <unordered_map>

class Player;
class EnemyComponent;
class FireParticleComponent;

enum class Stage {
	MAP1,
};
struct TileType {
	enum Type {
		GOAL = -1,
		WALL = 0,
		FLOOR = 1,
		RESOURCE = 2,
	};
};

struct CharacterType {
	enum Type {
		EMPTY = 0,
		PLAYER = -1,
		ENEMY = 1,
	};
};

class DungeonScene :
    public Scene
{
public:
    DungeonScene(class Game& game);

    void fastUpdateScene() override;
	void updateScene() override;
	void lateUpdateScene() override;
	void drawScene() override;
    void onEnter() override;
    void onExit() override;
    
    const std::string getName() const override {
        return "DUNGEON";
    };


    //エネミーの作成
    void createEnemy(const std::string& enemyID, float x, float y);
    //プレイヤーの作成
	void createPlayer(float x, float y);
    //リソースの作成
	void createResource(const std::string& resourceID, const std::string& meshID, float x, float y, int index);
	void spawnResource();
	void deleteResourceFromIndex(int index);

    //シーン遷移
    void returnToTown();
	void transitToGameOver();

    //エネミー配列の制御
	void addEnemy(EnemyComponent* enemy);
	void removeEnemy(EnemyComponent* enemy);
	void sortEnemiesByDistanceToPlayer();

	//パーティクル配列の制御
	void addParticle(FireParticleComponent* particle);
	void removeParticle(FireParticleComponent* particle);

	//ミニマップの更新
	void updateMiniMapPos();
	void updateMiniMapDirection();
	//UIの更新
	void updateHPUI();
	void updateAPUI();
	void updateItemUI();
	void updateItemFrame();
	void pushMessage(const std::string& message);
	void updateGold();

	//ターンの制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();

	//マップ移動
	void moveNextFloor();

    //setter
    //マップ情報
	void setTileDataAt(int x, int y, int data);
	void setTileDataAt(int index, int data);
	void setTileData(const std::vector<std::vector<int>>& mapData);
	void setCharacterDataAt(int x, int y, int data);
	void setCharacterDataAt(int index, int data);
	void setCharacterData(const std::vector<std::vector<int>>& objectData);
	void setMapSize(int size) { mMapSize = size; }

    //getter
    //マップ情報
	const int getMapSize() const { return mMapSize; }
	int getTileDataAt(int x, int y);
	int getTileDataAt(int index);   
	int getCharacterDataAt(int x, int y);
	int getCharacterDataAt(int index);
	const Stage& getStage();
    //エネミー
	const std::vector<EnemyComponent*>& getEnemies() const { return mEnemies; }
	const int getEnemyCount() const { return mEnemies.size(); }
	EnemyComponent* getEnemyFromIndexPos(int index);
	EnemyComponent* getEnemyFromIndexPos(int x, int y);
    //プレイヤー
	Player* getPlayer() const { return mPlayer; }
    int getPlayerActLimit();
    //リソース
    class Resource* getResource(int index);
    class Resource* getResource(int x, int y);
	//ターン情報
	enum struct TurnType getTurnType() const;
	//ダメージテキスト
	float getDamageTextNum() const;
	void createDamageText(const XMFLOAT3& pos, int digit);
	//ターンオブザーバー
	const class TurnObserver& getTurnObserver();

private:
	//マップ情報
    int mMapSize;
	std::vector<std::vector<int>> mTileData; //[x][y]
	std::vector<std::vector<int>> mCharacterData; //[x][y]
	Stage mStage;

	//キャラクター
	std::vector<EnemyComponent*> mEnemies;
	Player* mPlayer;
	//リソース
	std::unordered_map<int, class Resource*> mResources;
	int mMaxResourcePoint;

	//UIアクター
	class MiniMap* mMiniMap;
	class DungeonUI* mUI;

	//エフェクト
	std::vector<FireParticleComponent*> mParticles;
	
	//管理クラス
	std::unique_ptr<class MapGenerator> mMapGenerator;	//マップ生成
	std::unique_ptr<class TurnObserver > mTurnObserver;	//ターン管理
	std::unique_ptr<class DamageTextGenerator> mDamageTextManaager;	//ダメージテキスト管理
};

