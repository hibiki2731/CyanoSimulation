#pragma once
#include "Actor.h"

class TextComponent;
class SpriteComponent;
class DungeonScene;

class DungeonUI : public Actor
{
public:
	DungeonUI(DungeonScene& scene);
	void updateHP();
	void updateAP();
	void updateItemIcon();

private:
	TextComponent* mHPValueText;
	TextComponent* mAPValueText;
	SpriteComponent* mHPBar;
	XMFLOAT2 mHPBarOriginalSize;
	XMFLOAT2 mHPBarOffsetPos;
	SpriteComponent* mAPBar;
	XMFLOAT2 mAPBarOriginalSize;
	XMFLOAT2 mAPBarOffsetPos;
	SpriteComponent* mCanvas;
	DungeonScene& mDungeonScene;

	//アイテムアイコン
	XMFLOAT2 mItemIconOriginPos;
	static constexpr XMFLOAT2 ItemIconSize = {60.0f, 60.0f};
	std::vector<SpriteComponent*> mItemIcons;
};

