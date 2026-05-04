#pragma once
#include "Menu.h"

class InnMenu : public Menu {
public:
	InnMenu(TownScene& scene, float zDepth);
	DECLARE_CLASS_NAME(InnMenu)

	void selectedAct() override;
	void updateMenu() override;
	void inputMenu() override;
	void updateActor() override;
	void applyComponentLabel() override;

private:
	void stay();
	void save();
	void updateDescriptor();

	struct IXAudio2SourceVoice* mSleepVoice;
	class TextComponent* mDescriptor;
	float mDescriptorDefaultZ;
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
