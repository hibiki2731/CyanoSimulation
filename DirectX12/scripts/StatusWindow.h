#pragma once
#include "Actor.h"

class TownScene;
class PlayerManager;
class ItemManager;
class TextComponent;
class SpriteComponent;

class StatusWindow :
    public Actor
{
public:
	StatusWindow(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(StatusWindow)
	void updateStatus();

private:
	PlayerManager& mPlayerManager;
	ItemManager& mItemManager;
	TextComponent* mHPValueText;
	XMFLOAT2 mHPBarOriginalSize;
	SpriteComponent* mHPBar;
	TextComponent* mAPValueText;
	std::vector<SpriteComponent*> mItemIcons;
};

