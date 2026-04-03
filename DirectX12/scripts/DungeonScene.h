#pragma once
#include "Scene.h"
#include <memory>
#include <unordered_map>

class Player;
class EnemyComponent;

enum class Stage {
	MAP1,
};
struct TileType {
	enum Type {
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
    
    const std::string& getName() const override {
        return "DUNGEON";
    };


    //エネミーの作成
    void createEnemy(const std::string& enemyID, float x, float y);
    //プレイヤーの作成
	void createPlayer(float x, float y);
    //リソースの作成
	void createResource(const std::string& resourceID, const std::string& meshID, float x, float y, int index);

    //シーン遷移
    void returnToTown();

    //エネミー配列の制御
	void addEnemy(EnemyComponent* enemy);
	void removeEnemy(EnemyComponent* enemy);
	void sortEnemiesByDistanceToPlayer();

	//ミニマップの更新
	void updateMiniMapPos();
	void updateMiniMapDirection();

	//ダメージテキストの公人
	void updateDTView(XMMATRIX& view);

	//ターンの制御
	void moveToPlayerTurn();
	void moveToEnemyTurn();

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
    //エネミー
	const std::vector<EnemyComponent*>& getEnemies() const { return mEnemies; }
	const int getEnemyCount() const { return mEnemies.size(); }
	EnemyComponent* getEnemyFromIndexPos(int index);
	EnemyComponent* getEnemyFromIndexPos(int x, int y);
    //プレイヤー
	Player* getPlayer() const { return mPlayer; }
    int getPlayerActLimit();
    //リソース
    const std::string& getResourceID(int index);
    const std::string& getResourceID(int x, int y);
	//ターン情報
	TurnType getTurnType() const;
	//ダメージテキスト
	int getDamageTextNum() const;
	void createDamageText(const XMFLOAT3& pos, int digit);

private:
    int mMapSize;
	std::vector<std::vector<int>> mTileData; //[x][y]
	std::vector<std::vector<int>> mCharacterData; //[x][y]

	std::vector<EnemyComponent*> mEnemies;
    std::unique_ptr<class MapManager> mMapManager;
	std::unique_ptr<class DamageTextManager> mDamageTextManaager;
	Player* mPlayer;
	std::unordered_map<int, std::string> mResourceIDs;

	class MiniMap* mMiniMap;
};

