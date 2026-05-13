#pragma once
#include "Object.h"
#include <deque>

class TextComponent;
class SpriteComponent;
class DungeonScene;

class DungeonUI : public Object
{
public:
	DungeonUI(DungeonScene& scene);
	DECLARE_CLASS_NAME(DungeonUI)
	void applyComponentLabel() override;

	void updateHP();
	void updateAP();
	void updateItemIcon();
	void updateItemFrame();
	void pushMessage(const std::string& message);
	void updateGold();

private:
	//各種UIコンポーネント
	TextComponent* mHPValueText;
	TextComponent* mAPValueText;
	SpriteComponent* mHPBar;
	XMFLOAT2 mHPBarOriginalSize;
	SpriteComponent* mAPBar;
	XMFLOAT2 mAPBarOriginalSize;
	SpriteComponent* mCanvas;
	SpriteComponent* mItemSelectFrame;
	XMFLOAT3 mFrameOriginPos;
	TextComponent* mSelectItemText;
	TextComponent* mMessageText;
	std::deque<std::string> mMessages;
	const int mMaxMessageNum;
	TextComponent* mGoldText;


	//アイテムアイコン
	static XMFLOAT2 ItemIconOriginPos;
	static XMFLOAT2 ItemIconSize;
	std::vector<SpriteComponent*> mItemIcons;

	//参照
	class ItemManager& mItemManager;
	class PlayerManager& mPlayerManager;
	class Player& mPlayer;
	const struct PlayerData& mPlayerData;
};

