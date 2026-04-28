#pragma once
#include "Scene.h"
#include "Actor.h"


class TitleScene : public Scene
{
public:
	TitleScene(class Game& game);
	void onEnter() override;
	void onExit() override;

	const std::string& getName() const override {
		return "TITLE";
	};

private:
	class ItemManager& mItemManager;
	class PlayerManager& mPlayerManager;
	class AudioManager& mAudioManager;
};


class TitleUI : public Actor
{
public:
	TitleUI(TitleScene& scene);
	DECLARE_CLASS_NAME(TitleUI)

	void inputActor() override;
	void updateActor() override;

private:
	void startTransit();
	class TextComponent* mStartText;
	bool isStarting = false;
	struct IXAudio2SourceVoice* mStartSEVoice;
	int mTimer;
};
