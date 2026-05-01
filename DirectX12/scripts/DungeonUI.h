#pragma once
#include "Actor.h"
#include <deque>

class TextComponent;
class SpriteComponent;
class DungeonScene;

class DungeonUI : public Actor
{
public:
	DungeonUI(DungeonScene& scene);
	DECLARE_CLASS_NAME(DungeonUI)
	void updateHP();
	void updateAP();
	void updateItemIcon();
	void updateItemFrame();
	void pushMessage(const std::string& message);

private:
	//各種UIコンポーネント
	TextComponent* mHPValueText;
	TextComponent* mAPValueText;
	SpriteComponent* mHPBar;
	XMFLOAT2 mHPBarOriginalSize;
	XMFLOAT2 mHPBarOffsetPos;
	SpriteComponent* mAPBar;
	XMFLOAT2 mAPBarOriginalSize;
	XMFLOAT2 mAPBarOffsetPos;
	SpriteComponent* mCanvas;
	SpriteComponent* mItemSelectFrame;
	TextComponent* mSelectItemText;
	TextComponent* mMessageText;
	std::deque<std::string> mMessages;
	const int mMaxMessageNum;


	//アイテムアイコン
	static XMFLOAT2 ItemIconOriginPos;
	static XMFLOAT2 ItemIconSize;
	std::vector<SpriteComponent*> mItemIcons;

	//参照
	class ItemManager& mItemManager;
	class PlayerManager& mPlayerManager;
	class Player& mPlayer;
};

