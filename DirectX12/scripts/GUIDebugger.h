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

	//エディタGUIの描画開始
	void begin();
	//描画終わり
	void end();
	//プレイヤーの現在位置を表示
	void drawCameraPos();
	//オブジェクトの各種パラメータを操作するためのGUIを描画
	void drawObjectDebugGUI(std::vector<class Object*>& objects);

	//カメラ位置の更新
	void updateCameraPos(XMFLOAT3& position);

private:
	//ヒエラルキーウィンドウ
	void hierarchyMenu(std::vector<class Object*>& objects);
	//エディタウィンドウ
	void objectEditer(class Object* object, std::vector<class Object*>& objects);

	//GUIの各ウィジェット
	void inputName(class Object* obj, std::vector<class Object*>& objects );					//オブジェクトを変更
	void labelEditer(class Object* obj);														//ラベルとコンポーネントの対応付け
	void objectDeleteButton(class Object* obj);													//オブジェクトをJSON上のデータからも除去
	void objectHideButton(class Object* obj, std::vector<class Object*>& objects);				//オブジェクトをヒエラルキーウィンドウから除去
	void objectDuplicateButton(class Object* refObj, std::vector<class Object*>& objects );		//オブジェクトの複製用ボタン
	void objectDuplicate(const std::string& refID, std::vector<class Object*>& objects );		//オブジェクト複製関数
	void saveToSceneJsonButton(std::vector<class Object*>& objects);							//シーンの最初に生成するオブジェクトIDを保存
	void saveToObjectJsonButton(class Object* object);											//オブジェクトのパラメータをJSONに保存
	void componentEditer(class Object* object);													//各コンポーネント編集

	
	//---コンポーネントに対する操作---
	//新しくコンポーネントを追加する場合
	//editer : componentEditerに追加
	//add    : objectEditerに追加、componentListに型名を追加する
	//save	 : saveToObjectJsonButtonに追加
	//Object.cppのloadComponentDataに新たなコンポーネントの読み込みを追加する
	//meshComponent
	void meshComponentEditer(class Component* component);
	void addMeshComponent(class Object* object);
	void saveMeshComponent(class Component* component, nlohmann::json& objJson);

	//pointLightComponent
	void pointLightComponentEditer(class Component* component);
	void addPointLightComponent(class Object* object);
	void savePointLightComponent(class Component* component, nlohmann::json& objJson);

	//SpriteComponent
	void spriteComponentEditer(class Component* component);
	void addSpriteComponent(class Object* object);
	void saveSpriteComponent(class Component* component, nlohmann::json& objJson);

	//TextComponent
	void textComponentEditer(class Component* component);
	void addTextComponent(class Object* object);
	void saveTextComponent(class Component* component, nlohmann::json& objJson);

	//---メンバー変数---
	ComPtr<ID3D12DescriptorHeap> 	mSrvHeap;
	Graphic& 						mGraphic;
	class Game& 					mGame;
	//スプライト用
	std::string 					mSpriteFilePathBuffer = "default.png";
	//カメラの位置
	XMFLOAT3 						mCameraPos;
	
};
