#pragma once
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx12.h"
#include "Imgui/imgui_impl_win32.h"
#include "Graphic.h"

class GUIDebugger
{
public:
	GUIDebugger(class Game& game);
	~GUIDebugger();

	void begin();
	void end();
	void drawSpriteDebugGUI(class SpriteComponent& sprite);
	void drawTextDebugGUI(class TextComponent& text);
	void drawCameraPos();
	void drawObjectDebugGUI(std::vector<class Object*>& objects);

	//カメラ位置の更新
	void updateCameraPos(XMFLOAT3& position);

private:
	void objectDeleteButton(class Object* obj);
	void objectDuplicateButton(class Object* refObj);
	void saveToJson(std::vector<class Object*>& objects);


	ComPtr<ID3D12DescriptorHeap> mSrvHeap;
	Graphic& mGraphic;
	class Game& mGame;
	//スプライト用
	std::string mSpriteFilePathBuffer = "default.png";
	//カメラの位置
	XMFLOAT3 mCameraPos;
	
};
