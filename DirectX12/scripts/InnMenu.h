#pragma once
#include "Menu.h"

class InnMenu : public Menu {
public:
	InnMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(InnMenu)

	void selectedAct() override;
	void updateMenu() override;

private:
	void stay();
	void save();

	struct IXAudio2SourceVoice* mSleepVoice;
	bool isFading;
};

class ConfirmWindow : public Menu{
public:
	ConfirmWindow(TownScene& scene, InnMenu& menu);
	DECLARE_CLASS_NAME(ConfirmWindow)

	void inputMenu() override;
	void selectedAct() override;

private:
	void save();
};
