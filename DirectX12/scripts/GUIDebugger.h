#pragma once
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_dx12.h"
#include "Imgui/imgui_impl_win32.h"
#include "Graphic.h"
#include "json_fwd.hpp"

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
	//ヒエラルキー
	void hierarchyMenu(std::vector<class Object*>& objects);
	//オブジェクトエディタ
	void objectEditer(class Object* object, std::vector<class Object*>& objects);

	void inputName(class Object* obj, std::vector<class Object*>& objects );
	void labelEditer(class Object* obj);
	void objectDeleteButton(class Object* obj);
	void objectHideButton(class Object* obj, std::vector<class Object*>& objects);
	void objectDuplicateButton(class Object* refObj, std::vector<class Object*>& objects );
	void objectDuplicate(const std::string& refID, std::vector<class Object*>& objects );
	void saveToSceneJsonButton(std::vector<class Object*>& objects);
	void saveToObjectJsonButton(class Object* object);
	void componentEditer(class Object* object);

	//meshComponent
	void meshComponentEditer(class Component* component);
	void addMeshComponent(class Object* object);
	void saveMeshComponent(class Component* component, nlohmann::json& objJson);

	//pointLightComponent
	void pointLightComponentEditer(class Component* component);
	void addPointLightComponent(class Object* object);
	void savePointLightComponent(class Component* component, nlohmann::json& objJson);

	//fireParticleComponent
	void fireParticleComponentEditer(class Component* component);
	void addFireParticleComponent(class Object* object);
	void saveFireParticleComponent(class Component* component, nlohmann::json& objJson);

	//SpriteComponent
	void spriteComponentEditer(class Component* component);
	void addSpriteComponent(class Object* object);
	void saveSpriteComponent(class Component* component, nlohmann::json& objJson);

	//TextComponent
	void textComponentEditer(class Component* component);
	void addTextComponent(class Object* object);
	void saveTextComponent(class Component* component, nlohmann::json& objJson);

	ComPtr<ID3D12DescriptorHeap> mSrvHeap;
	Graphic& mGraphic;
	class Game& mGame;
	//スプライト用
	std::string mSpriteFilePathBuffer = "default.png";
	//カメラの位置
	XMFLOAT3 mCameraPos;
	
};
