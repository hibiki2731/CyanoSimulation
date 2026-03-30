#pragma once
#include "Actor.h"
#include <vector>

class SpriteComponent;
class MapManager;
class Game;

class MiniMap :public Actor
{
public:
	MiniMap(Game& game, MapManager& mapManager);
	void updatePosition();
	void updateDirection();

private:
	MapManager& mMapManager;
	SpriteComponent* mCanvas;
	SpriteComponent* mPlayerIcon;
	std::vector<SpriteComponent*> mTileIcon;
	std::vector<SpriteComponent*> mEnemyIcon;
	std::vector<SpriteComponent*> mResourceIcon;
	XMFLOAT3 mMiniMapPosition;

};

