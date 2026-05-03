#pragma once
#include "Object.h"

class TownScene;
class PlayerManager;
class ItemManager;
class TextComponent;
class SpriteComponent;

class StatusWindow :
    public Object
{
public:
	StatusWindow(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(StatusWindow)

	void applyComponentLabel() override;
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

