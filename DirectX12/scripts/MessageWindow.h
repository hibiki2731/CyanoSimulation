#pragma once
#include <string>
#include "Actor.h"

class TextComponent;
class Player;
class PlayerManager;

class MessageWindow : public Actor
{
public:
	MessageWindow(Game& game);
	~MessageWindow() {};

	void setPlayer(Player* player);
	void setPlayerManager(PlayerManager* playerManager);

	void updateActor() override;

private:

	std::wstring mMessage;
	Player* mPlayer;
	PlayerManager* mPlayerManager;
	TextComponent* mText;
};

