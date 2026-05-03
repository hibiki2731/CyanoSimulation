#include "GUIDebugger.h"
#include "Game.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "fstream"
#include "Scene.h"
#include "Actor.h"
#include "Component.h"
#include "PointLightComponent.h"
#include "FireParticleComponent.h"
#include "MeshComponent.h"
#include "Imgui/imgui_stdlib.h"
#include "MyUtility.h"
#include "myJson.h"
#include "unordered_map"
#include "Object.h"
#include "CameraComponent.h"

struct ExampleDescriptorHeapAllocator
{
	ID3D12DescriptorHeap* Heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
	UINT                        HeapHandleIncrement;
	ImVector<int>               FreeIndices;

	void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
	{
		IM_ASSERT(Heap == nullptr && FreeIndices.empty());
		Heap = heap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
		HeapType = desc.Type;
		HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
		HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
		HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
		FreeIndices.reserve((int)desc.NumDescriptors);
		for (int n = desc.NumDescriptors; n > 0; n--)
			FreeIndices.push_back(n - 1);
	}
	void Destroy()
	{
		Heap = nullptr;
		FreeIndices.clear();
	}
	void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
	{
		IM_ASSERT(FreeIndices.Size > 0);
		int idx = FreeIndices.back();
		FreeIndices.pop_back();
		out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
		out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
	}
	void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
	{
		int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
		int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
		IM_ASSERT(cpu_idx == gpu_idx);
		FreeIndices.push_back(cpu_idx);
	}
};

static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;

GUIDebugger::GUIDebugger(Game& game)
	: mGraphic(game.getGraphic()),
	mGame(game)
{
	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	//imGui用のSRVヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.NodeMask = 0;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HRESULT hr = mGraphic.getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mSrvHeap.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));
	g_pd3dSrvDescHeapAlloc.Create(mGraphic.getDevice(), mSrvHeap.Get());


	//ImGuiコンテキストの作成
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//インプットの設定
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	//style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	//Win32バックエンドの初期化
	ImGui_ImplWin32_Init(mGraphic.getWindowHandle());

	//DirectX12バックエンドの初期化
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = mGraphic.getDevice();
	init_info.CommandQueue = mGraphic.getCommandQueue();
	init_info.NumFramesInFlight = Graphic::FrameCount;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
	init_info.SrvDescriptorHeap = mSrvHeap.Get();
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };

	ImGui_ImplDX12_Init(&init_info);

	mCameraPos = { 0.0f, 0.0f, 0.0f };
}

GUIDebugger::~GUIDebugger()
{
	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void GUIDebugger::begin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void GUIDebugger::end()
{
	ImGui::Render();
	mGraphic.getCommandList()->SetDescriptorHeaps(1, mSrvHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mGraphic.getCommandList());
}

void GUIDebugger::drawSpriteDebugGUI(SpriteComponent& sprite)
{
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(sprite.mStructName.c_str())) {
		ImGui::SliderFloat("X", &sprite.mPosition.x, 0.0f, Graphic::ClientWidth);
		ImGui::SliderFloat("Y", &sprite.mPosition.y, 0.0f, Graphic::ClientHeight);
		ImGui::SliderFloat("Width", &sprite.mSpriteSize.x, 0.01f, Graphic::ClientWidth);
		ImGui::SliderFloat("Height", &sprite.mSpriteSize.y, 0.1f, Graphic::ClientHeight);
		ImGui::SliderFloat("BorderSize", &sprite.mBordarSize, 0.0f, 100.0f);
		ImGui::SliderFloat("Rotation", &sprite.mRotation, 0.0f, 2.0f * XM_PI);

		//テクスチャの変更
		ImGui::InputText("Texture Path", &sprite.mTextureFilePath);
		ImGui::SameLine();

		if (ImGui::Button("Apply")) {
			sprite.create(sprite.mTextureFilePath);
		}

		if (ImGui::Button("Save to Json")) {
			std::ifstream infile("assets/data/spriteData.json");
			nlohmann::json j;
			infile >> j;
			j[sprite.mStructName]["x"] = sprite.mPosition.x;
			j[sprite.mStructName]["y"] = sprite.mPosition.y;
			j[sprite.mStructName]["width"] = sprite.mSpriteSize.x;
			j[sprite.mStructName]["height"] = sprite.mSpriteSize.y;
			j[sprite.mStructName]["borderSize"] = sprite.mBordarSize;
			j[sprite.mStructName]["rotation"] = sprite.mRotation;
			j[sprite.mStructName]["filePath"] = sprite.mTextureFilePath;

			std::ofstream outfile("assets/data/spriteData.json");
			outfile << j.dump(4); // 4はインデントスペースの数			
		}

	}
	ImGui::End();
}

void GUIDebugger::drawTextDebugGUI(TextComponent& text)
{
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

	if (ImGui::Begin(text.mStructName.c_str())) {
		if (ImGui::SliderFloat("Position", &text.mPosition.x, 0.0f, Graphic::ClientWidth)) {
			text.applyTextTexture();
		}
		if (ImGui::Button("Center")) {
			text.setPosition(text.mPosition.x - text.mTextWidth / 2.0f, text.mPosition.y);
		}
		if (ImGui::ColorEdit4("Color", &text.mTextColor.r)) {
			//ブラシを更新
			HRESULT hr = mGraphic.getD2DDeviceContext()->CreateSolidColorBrush(text.mTextColor, &text.mTextBrush);
			assert(SUCCEEDED(hr));
			text.applyTextTexture();
		}
		;
		if (ImGui::SliderFloat("FontSize", &text.mFontSize, 0.01f, Graphic::ClientWidth)) {
			text.applyTextFormat();
		}
		if (ImGui::SliderFloat("LineSpace", &text.mLineSpace, 0.0f, 100.0f)) {
			text.mBaseLineSpace = text.mLineSpace * 0.8f;
			text.applyTextFormat();
		}

		//テキストの変更
		ImGui::InputText("Text", &text.mTextBuffer);

		if (ImGui::Button("Apply")) {
			text.mText = Utility::stringToWString(text.mTextBuffer);
			text.applyTextTexture();
		}

		if (ImGui::Button("Save to Json")) {
			std::ifstream infile("assets/data/textData.json");
			nlohmann::json j;
			infile >> j;
			//j[text.mStructName]["x"] = text.mPosX;
			//j[text.mStructName]["y"] = text.mPosY;
			j[text.mStructName]["fontSize"] = text.mFontSize;
			j[text.mStructName]["lineSpace"] = text.mLineSpace;
			j[text.mStructName]["text"] = Utility::wstringToString(text.mText);
			j[text.mStructName]["color"] = text.mColorFloat;

			std::ofstream outfile("assets/data/textData.json");
			outfile << j.dump(4); // 4はインデントスペースの数			
		}

	}
	ImGui::End();
}

void GUIDebugger::drawCameraPos()
{
	ImGui::SetNextWindowSize(ImVec2(400, 50), ImGuiCond_Appearing);
	ImGui::Begin("CameraPos");

	//カメラの位置を表示
	std::string text = "x: " + std::to_string(mCameraPos.x) + " y: " + std::to_string(mCameraPos.y) + " z: " + std::to_string(mCameraPos.z) + "\n";
	ImGui::Text(text.c_str());

	ImGui::End();

}

//シーン中のアクターを取得
static int selectedIndex = -1;          //選択中のアクターインデックス
static bool showEditorWindow = false;   //エディタウィンドウを表示するかどうか

//追加可能コンポーネント
static std::vector<std::string> componentList = { "MeshComponent", "PointLightComponent", "FireParticleComponent", "SpriteComponent", "TextComponent"};
static int selectedComponentIndex = 0;

void GUIDebugger::drawObjectDebugGUI(std::vector<Object*>& objects)
{

	//ヒエラルキーウィンドウ
	hierarchyMenu(objects);

	//エディタウィンドウ
	if (showEditorWindow && selectedIndex >= 0 && selectedIndex < objects.size()) 
		objectEditer(objects[selectedIndex], objects);

}

void GUIDebugger::updateCameraPos(XMFLOAT3& position)
{
	mCameraPos = position;
}

void GUIDebugger::hierarchyMenu(std::vector<class Object*>& objects)
{
	//ウィンドウの大きさ
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Appearing);
	//アクターリストウィンドウ
	ImGui::Begin("Scene Hierarchy");


	//オブジェクトの表示
	for (int i = 0; i < objects.size(); i++) {

		ImGui::PushID(i);
		//選択状態を判定してハイライトをつける
		bool isSelected = (selectedIndex == i);

		std::string labelName = objects[i]->mName.empty() ? "(Unnamed)" : objects[i]->mName;

		if (ImGui::Selectable(labelName.c_str(), isSelected)) {
			selectedIndex = i;          //選択インデックスを更新
			showEditorWindow = true;    //エディタウィンドウのフラグを立てる
		}

		ImGui::PopID();
	}

	//オブジェクトの追加
	if (ImGui::Button("AddObject")) {
		auto object = std::make_unique<Object>(mGame.getSceneManager().getCurrentScene(), "EmptyObject");

		//配列へ追加
		mGame.getSceneManager().getCurrentScene().addActor(std::move(object));
	}

	//jsonへ保存
	saveToSceneJsonButton(objects);
	ImGui::End();
}

void GUIDebugger::objectEditer(Object* object, std::vector<class Object*>& objects)
{

		//ウィンドウの大きさ
		ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
		ImGui::Begin("Editor", &showEditorWindow);

		ImGui::Text(("Editing: " + object->getClassName()).c_str());
		//複製ボタン
		ImGui::SameLine();
		objectDuplicateButton(object, objects);
		//隠すボタン
		ImGui::SameLine();
		objectHideButton(object, objects);
		//削除ボタン
		ImGui::SameLine();
		objectDeleteButton(object);
		ImGui::Separator();

		//オブジェクトの名前
		inputName(object, objects);
		//位置
		ImGui::DragFloat3("Position", &object->mPosition.x, 0.1f);
		//回転
		ImGui::DragFloat3("Rotation", &object->mRotation.x, 0.1f);
		//スケール
		ImGui::DragFloat3("Scale", &object->mScale.x, 0.1f);

		//ラベルエディタ
		labelEditer(object);

		//コンポーネント追加用ウィジェット
		ImGui::Separator();
		if (selectedComponentIndex >= 0 &&
			selectedComponentIndex < componentList.size() &&
			ImGui::BeginCombo("Components", componentList[selectedComponentIndex].c_str())) {
			for (int n = 0; n < componentList.size(); n++) {
				const bool is_selected = (selectedComponentIndex == n);

				if (ImGui::Selectable(componentList[n].c_str(), is_selected)) {
					selectedComponentIndex = n;
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();

				}
			}
			ImGui::EndCombo();
		}
		//コンポーネント追加ボタン
		if (ImGui::Button("addComponent")) {
			const std::string& componentName = componentList[selectedComponentIndex];

			if (componentName == "MeshComponent")				addMeshComponent(object);
			else if (componentName == "PointLightComponent")	addPointLightComponent(object);
			else if (componentName == "FireParticleComponent")	addFireParticleComponent(object);
			else if (componentName == "SpriteComponent")	addSpriteComponent(object);
			else if (componentName == "TextComponent")	addTextComponent(object);
		}

		//コンポーネントの編集
		componentEditer(object);

		//セーブ
		saveToObjectJsonButton(object);
		ImGui::End();
}

void GUIDebugger::inputName(Object* obj, std::vector<class Object*>& objects)
{
	static std::string newName;
	ImGui::InputText("Name", &newName);
	ImGui::SameLine();
	static bool isError = false;
	if (ImGui::Button("apply")) {

		//名前が一致するオブジェクトが存在するか確認
		//オブジェクトデータの取得
		std::ifstream infile("assets/data/objectData.json");
		nlohmann::json j;
		infile >> j;
		std::string sceneName = mGame.getSceneManager().getCurrentScene().getName();

		//名前の一致するオブジェクトがあるか確認
		if (j[sceneName].contains(newName) && j[sceneName][newName].size() != 0) {
			//一致した場合
			isError = true;
		}
		else {
			//一致しなかった場合
			isError = false;

			//シーンの初期オブジェクトに対する処理
			std::ifstream insceneFile("assets/data/sceneData.json");
			nlohmann::json sceneJson;
			insceneFile >> sceneJson;
			std::vector<std::string> objects = sceneJson[sceneName].get<std::vector<std::string>>();
			for (int i = 0; i < objects.size(); i++) {
				//初期オブジェクトに含まれる場合
				if (objects[i] == obj->mName) {
					objects[i] = newName;

					sceneJson[sceneName] = std::move(objects);

					std::ofstream outsceneFile("assets/data/sceneData.json");
					outsceneFile << sceneJson.dump(4);
					break;
				}
			}

			//重複がなかった場合の処理
			//新たなオブジェクトIDに元データを移す
			j[sceneName][newName] = std::move(j[sceneName].value(obj->mName, R"({})"_json));
			//元オブジェクトIDをjsonから削除
			if (j[sceneName].contains(obj->mName)) j[sceneName].erase(obj->mName);
			obj->mName = newName;

			//オブジェクトファイルに保存
			std::ofstream outfile("assets/data/objectData.json");
			outfile << j.dump(4);
		}
	}

	if (isError) {
		ImGui::TextColored((ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), "the name is conflicted");
		ImGui::SameLine();
		if (ImGui::Button("do you duplicate?")) {
			obj->mState = Actor::State::Dead;
			objectDuplicate(newName, objects);
		}
	}

}

void GUIDebugger::labelEditer(Object* obj)
{
	ImGui::Separator();
	//ラベルを取得
	auto& componentLabels = obj->mComponentLabels;
	for (auto& [key, label] : componentLabels) {

		//ラベル名の取得
		std::string selectedComponentName;
		if (label.pComponent != nullptr) 
			selectedComponentName = label.pComponent->getComponentName();
		else
			selectedComponentName = "None";

		if (ImGui::BeginCombo(key.c_str(), selectedComponentName.c_str())) {
			for (int i = 0; i < obj->mComponents.size(); i++) {
				//ラベル付したいコンポーネントの型と一致しているか
				if (label.componentName != obj->mComponents[i]->getComponentName()) continue;
				//コンポーネント名によるコンフリクトを避けるためのIDをpush
				ImGui::PushID(i);

				const bool is_selected = (obj->mComponents[i].get() == label.pComponent);

				if (ImGui::Selectable(obj->mComponents[i]->getComponentName().c_str(), is_selected)) {
					label.pComponent = obj->mComponents[i].get();
					obj->applyComponentLabel();
				}
				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}

	}
}

void GUIDebugger::objectDeleteButton(Object* obj)
{
	if (ImGui::Button("Delete")) {
		//シーンの取得
		const std::string& sceneName = mGame.getSceneManager().getCurrentScene().getName();

		//jsonに反映
		std::ifstream infile("assets/data/objectData.json");
		nlohmann::json j;
		infile >> j;
		j[sceneName].erase(obj->mName);

		std::ofstream outfile("assets/data/objectData.json");
		outfile << j.dump(4);

		//シーンの初期に存在する場合
		std::ifstream insceneFile("assets/data/sceneData.json");
		nlohmann::json sceneJson;
		insceneFile >> sceneJson;
		std::vector<std::string> objects = sceneJson[sceneName].get<std::vector<std::string>>();
		for (int i = 0; i < objects.size(); i++) {
			//初期オブジェクトに含まれる場合
			if (objects[i] == obj->mName) {

				//配列から除去
				objects.erase(objects.begin() + i);
				sceneJson[sceneName] = objects;
				std::ofstream outsceneFile("assets/data/sceneData.json");
				outsceneFile << sceneJson.dump(4);
				break;
			}
		}

		//アクターを死亡させる
		obj->mState = Actor::State::Dead;

	}
}

void GUIDebugger::objectHideButton(Object* obj, std::vector<Object*>& objects)
{
	if (ImGui::Button("Hide")) {
		//ゲーム上から削除　オブジェクトデータは書き換えない
		obj->mState = Actor::State::Dead;
		std::erase_if(objects, [obj](const Object* o) {
			return obj->mName == o->mName;
			});
	}
}

//各ウィンドウの定義用関数
void GUIDebugger::objectDuplicateButton(Object* refObj, std::vector<Object*>& objects) {
	if (ImGui::Button("Duplicate")) {
		objectDuplicate(refObj->getName(), objects);
	}
}

void GUIDebugger::objectDuplicate(const std::string& refID, std::vector<Object*>& objects)
{

	auto& scene = mGame.getSceneManager().getCurrentScene();
	//現在、シーン中にオブジェクトが存在しているか判定
	std::ifstream infile("assets/data/objectData.json");
	nlohmann::json j;
	infile >> j;
	static bool dupFlag;
	dupFlag = true;
	static std::string newName;
	newName = refID;
	while (dupFlag) {
		for (auto obj : objects) {
			if (obj->getName() == newName) {
				dupFlag = false;
				break;
			}
		}
		if (!dupFlag) {
			//名前の語尾に_copyを追加
			newName += "_copy";
			dupFlag = true;
			continue;
		}

		//名前が変更された場合
		if (refID != newName) {
			//変更された名前がオブジェクトファイルにすでに登録されていたら、もう一度名前を変える
			if (j[scene.getName()].contains(newName) && j[scene.getName()][newName].size() != 0) {
				newName += "_copy";
				continue;
			}

			//オブジェクトの生成
			auto newObj = std::make_unique<Object>(scene, newName);
			//jsonへ反映
			j[scene.getName()][newName] = j[scene.getName()][refID];
			//配列へ追加
			mGame.getSceneManager().getCurrentScene().addActor(std::move(newObj));

			//オブジェクトファイルに保存
			std::ofstream outfile("assets/data/objectData.json");
			outfile << j.dump(4);

			return;
		}

		dupFlag = false;
	}

	//オブジェクトの生成
	auto newObj = std::make_unique<Object>(scene, refID);
	//配列へ追加
	mGame.getSceneManager().getCurrentScene().addActor(std::move(newObj));

}

void GUIDebugger::saveToSceneJsonButton(std::vector<Object*>& objects)
{
	if (ImGui::Button("Save Scene")) {
		std::ifstream infile("assets/data/sceneData.json");
		nlohmann::json j;
		infile >> j;

		std::string sceneName = mGame.getSceneManager().getCurrentScene().getName();
		std::vector<std::string> objectIDs;
		for (auto obj : objects) {
			objectIDs.push_back(obj->getName());
		}
		j[sceneName] = objectIDs;

		std::ofstream outfile("assets/data/sceneData.json");
		outfile << j.dump(4); // 4はインデントスペースの数	
	}
}

void GUIDebugger::saveToObjectJsonButton(Object* object)
{
	ImGui::Separator();
	if (ImGui::Button("Save Object")) {
		std::ifstream infile("assets/data/objectData.json");
		nlohmann::json j;
		infile >> j;
		std::string sceneName = mGame.getSceneManager().getCurrentScene().getName();
		nlohmann::json& objJson = j[sceneName][object->getName()];

		//アクターデータ
		objJson["position"] = object->mPosition;
		objJson["rotation"] = object->mRotation;
		objJson["scale"] = object->mScale;

		//ラベルの保存
		std::vector<int> labelIDs;
		objJson["label"].clear();
		for (auto& [key, label] : object->mComponentLabels) {
			//ラベルが設定されていない場合
			if (label.pComponent == nullptr) {
				objJson["label"][key]["id"] = -1;
				objJson["label"][key]["componentName"] = label.componentName;
				continue;
			}

			//インデックスを探索
			for (int i = 0; i < object->mComponents.size(); i++) {
				if (label.pComponent == object->mComponents[i].get()) {
					objJson["label"][key]["id"] = i;
					break;
				}
			}
			objJson["label"][key]["componentName"] = label.componentName;
		}

		//コンポーネントデータ
		int componentIndex = 0;
		objJson["components"].clear();
		for (int i = 0; i < object->mComponents.size(); i++) {
			std::string componentName = object->mComponents[i]->getComponentName();

			//MeshComponent
			if (componentName == "MeshComponent") {
				saveMeshComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			//PointLightComponent
			else if (componentName == "PointLightComponent") {
				savePointLightComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			//FireParticleComponent
			else if (componentName == "FireParticleComponent") {
				saveFireParticleComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			//SpriteComponent
			else if (componentName == "SpriteComponent") {
				saveSpriteComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			//TextComponent
			else if (componentName == "TextComponent") {
				saveTextComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
		}

		std::ofstream outfile("assets/data/objectData.json");
		outfile << j.dump(4);
		infile.close();
		outfile.close();

		mGame.getAssetManager().loadObjectJson();
	} 

}

void GUIDebugger::componentEditer(Object* object)
{
	//コンポーネントの設定
	for (int i = 0; i < object->mComponents.size(); i++) {
		//ハッシュのコンフリクトを防ぐため、IDをプッシュする
		ImGui::PushID(i);

		auto component = object->mComponents[i].get();
		static std::string componentName;
		componentName = component->getComponentName();

		//コンポーネントの名前表示
		ImGui::Separator();
		ImGui::Text(component->getComponentName().c_str());
		//各種コンポーネントの編集
		if (componentName == "MeshComponent")				 meshComponentEditer(component);
		else if (componentName == "PointLightComponent")	 pointLightComponentEditer(component);
		else if (componentName == "FireParticleComponent")	 fireParticleComponentEditer(component);
		else if (componentName == "SpriteComponent")		 spriteComponentEditer(component);
		else if (componentName == "TextComponent")			 textComponentEditer(component);

		//コンポーネントの除去
		if (ImGui::Button("RemoveComponent")) {
			object->removeComponent(component);
		}

		//IDのポップ
		ImGui::PopID();
	}
}

void GUIDebugger::meshComponentEditer(Component* component)
{
			//コンポーネントをキャスト
			MeshComponent* mesh = static_cast<MeshComponent*>(component);

			std::string meshID = mesh->getMeshID();
			//メッシュが設定されていない場合
			if (meshID == "NONE") {
				//メッシュIDの入力
				static std::string preMeshID;
				ImGui::InputText("##meshID", &preMeshID);
				ImGui::SameLine();
				if (ImGui::Button("Create")) {
					mesh->create(preMeshID);
				}

			}
			//メッシュが設定されている場合
			else ImGui::Text(meshID.c_str());
}

void GUIDebugger::addMeshComponent(Object* object)
{
	auto mesh = std::make_unique<MeshComponent>(*object);
	object->addComponent(std::move(mesh));
}

void GUIDebugger::saveMeshComponent(Component* component, nlohmann::json& objJson)
{
	//キャスト
	auto mesh = static_cast<MeshComponent*>(component);
	objJson["name"] = component->getComponentName();
	objJson["meshID"] = mesh->getMeshID();
}

void GUIDebugger::pointLightComponentEditer(Component* component)
{
	//コンポーネントをキャスト
	PointLightComponent* light = static_cast<PointLightComponent*>(component);

	//オフセット位置
	ImGui::DragFloat4("LightOffsetPos", &light->mOffsetPos.x, 0.05f);
	//色
	ImGui::ColorEdit4("LightColor", &light->mColor.x);
	//強度
	ImGui::DragFloat("Intensity", &light->mIntensity);
	//範囲
	ImGui::DragFloat("Range", &light->mRange);
}

void GUIDebugger::addPointLightComponent(Object* object)
{
	auto light = std::make_unique<PointLightComponent>(*object);
	light->setActive(true);
	object->addComponent(std::move(light));
}

void GUIDebugger::savePointLightComponent(Component* component, nlohmann::json& objJson)
{
	//キャスト
	auto light = static_cast<PointLightComponent*>(component);

	//データの格納
	objJson["name"] = component->getComponentName();
	objJson["lightOffsetPos"] = light->mOffsetPos;
	objJson["lightColor"] = light->mColor;
	objJson["intensity"] = light->mIntensity;
	objJson["range"] = light->mRange;
}

void GUIDebugger::fireParticleComponentEditer(Component* component)
{
	//コンポーネントをキャスト
	FireParticleComponent* fire = static_cast<FireParticleComponent*>(component);

	//エミット位置
	ImGui::DragFloat3("ParticleEmitPos", &fire->mEmitterPosition.x);
}

void GUIDebugger::addFireParticleComponent(Object* object)
{
	auto fire = std::make_unique<FireParticleComponent>(*object);
	object->addComponent(std::move(fire));
}

void GUIDebugger::saveFireParticleComponent(Component* component, nlohmann::json& objJson)
{
	auto fire = static_cast<FireParticleComponent*>(component);

	//データの格納
	objJson["name"] = component->getComponentName();
	objJson["particleEmitPos"] = fire->mEmitterPosition;
}

void GUIDebugger::spriteComponentEditer(Component* component)
{
	auto sprite = static_cast<SpriteComponent*>(component);

	ImGui::DragFloat3("Position", &sprite->mPosition.x, 0.5f);
	ImGui::SliderFloat("Width", &sprite->mSpriteSize.x, 0.01f, Graphic::ClientWidth);
	ImGui::SliderFloat("Height", &sprite->mSpriteSize.y, 0.1f, Graphic::ClientHeight);
	ImGui::SliderFloat("BorderSize", &sprite->mBordarSize, 0.0f, 100.0f);
	ImGui::SliderFloat("Rotation", &sprite->mRotation, 0.0f, 2.0f * XM_PI);

	//テクスチャの変更
	ImGui::InputText("Texture Path", &sprite->mTextureFilePath);
	ImGui::SameLine();

	if (ImGui::Button("Apply")) {
		sprite->create(sprite->mTextureFilePath);
	}
}

void GUIDebugger::addSpriteComponent(Object* object)
{
	auto sprite = std::make_unique<SpriteComponent>(*object);
	object->addComponent(std::move(sprite));
}

void GUIDebugger::saveSpriteComponent(Component* component, nlohmann::json& objJson)
{
	auto sprite = static_cast<SpriteComponent*>(component);
	objJson["name"] = "SpriteComponent";
	objJson["position"] = sprite->mPosition;
	objJson["width"] = sprite->mSpriteSize.x;
	objJson["height"] = sprite->mSpriteSize.y;
	objJson["borderSize"] = sprite->mBordarSize;
	objJson["rotation"] = sprite->mRotation;
	objJson["filePath"] = sprite->mTextureFilePath;

}

void GUIDebugger::textComponentEditer(Component* component)
{
	auto text = static_cast<TextComponent*>(component);
		if (ImGui::SliderFloat3("Position", &text->mPosition.x, 0.0f, Graphic::ClientWidth)) {
			text->applyTextTexture();
		}
		if (ImGui::Button("Center")) {
			text->setPosition(text->mPosition.x - text->mTextWidth / 2.0f, text->mPosition.y);
		}
		if (ImGui::ColorEdit4("Color", &text->mTextColor.r)) {
			//ブラシを更新
			HRESULT hr = mGraphic.getD2DDeviceContext()->CreateSolidColorBrush(text->mTextColor, &text->mTextBrush);
			assert(SUCCEEDED(hr));
			text->applyTextTexture();
		}
		;
		if (ImGui::SliderFloat("FontSize", &text->mFontSize, 0.01f, Graphic::ClientWidth)) {
			text->applyTextFormat();
		}
		if (ImGui::SliderFloat("LineSpace", &text->mLineSpace, 0.0f, 100.0f)) {
			text->mBaseLineSpace = text->mLineSpace * 0.8f;
			text->applyTextFormat();
		}

		//テキストの変更
		ImGui::InputText("Text", &text->mTextBuffer);

		if (ImGui::Button("Apply")) {
			text->mText = Utility::stringToWString(text->mTextBuffer);
			text->applyTextTexture();
		}
}

void GUIDebugger::addTextComponent(Object* object)
{
	auto text = std::make_unique<TextComponent>(*object);
	object->addComponent(std::move(text));
}

void GUIDebugger::saveTextComponent(Component* component, nlohmann::json& objJson)
{
	auto text = static_cast<TextComponent*>(component);
	objJson["name"] = "TextComponent";
	objJson["position"] = text->mPosition;
	objJson["fontSize"] = text->mFontSize;
	objJson["lineSpace"] = text->mLineSpace;
	objJson["text"] = Utility::wstringToString(text->mText);
	objJson["color"] = text->mTextColor;
}
