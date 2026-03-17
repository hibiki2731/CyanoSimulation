#pragma once
#include <string>
#include "Actor.h"

class TextComponent;
class Player;

class MessageWindow : public Actor
{
public:
	MessageWindow(Game* game);
	~MessageWindow() {};

	void inputActor() override;
	void updateActor() override;

private:

	std::wstring mMessage;
	TextComponent* mText;
};

