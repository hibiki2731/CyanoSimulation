#pragma once
#include "Actor.h"
#include <vector>

class SpriteComponent;
class DugeonScene;

class MiniMap :public Actor
{
public:
	MiniMap(DungeonScene& scene);
	void updatePosition();
	void updateDirection();

private:
	DungeonScene& mScene;
	SpriteComponent* mCanvas;
	SpriteComponent* mPlayerIcon;
	std::vector<SpriteComponent*> mTileIcon;
	std::vector<SpriteComponent*> mEnemyIcon;
	std::vector<SpriteComponent*> mResourceIcon;
	XMFLOAT3 mMiniMapPosition;

};

