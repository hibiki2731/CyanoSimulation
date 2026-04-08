#pragma once
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx12.h"
#include "Imgui/imgui_impl_win32.h"
#include "Graphic.h"

class GUIDebugger
{
public:
	GUIDebugger(Graphic& graphic);
	~GUIDebugger();

	void begin();
	void end();
	void drawSpriteDebugGUI(class SpriteComponent& sprite);
private:
	ComPtr<ID3D12DescriptorHeap> mSrvHeap;
	Graphic& mGraphic;
	//スプライト用
	const char* mSpriteFilePathBuffer = "default.png";
	
};
