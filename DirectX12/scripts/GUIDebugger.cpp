#include "GUIDebugger.h"
#include "Game.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "fstream"
#include "Scene.h"
#include "Actor.h"
#include "Component.h"
#include "Imgui/imgui_stdlib.h"
#include "MyUtility.h"
#include "myJson.h"
#include "unordered_map"
#include "Object.h"
#include "MeshComponent.h"
#include "CyanoSimulator.h"

//ImGUi用に必要なアロケーター
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
	//プロセスをDPI対応にし、メインモニターのスケールを取得
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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     //キーボード入力を有効化
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      //ゲームパッド入力を有効化

	//スケールの設定
	ImGuiStyle& style = ImGui::GetStyle();
	style.FontScaleDpi = main_scale; 
	
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
	mCyanoIdx = 0;
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
	//GUIを表示するのに毎フレーム必要な初期化処理
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void GUIDebugger::end()
{
	//GUIを表示するのに毎フレーム必要な終了処理
	ImGui::Render();
	mGraphic.getCommandList()->SetDescriptorHeaps(1, mSrvHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mGraphic.getCommandList());
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

void GUIDebugger::setCyanoSimulator(CyanoSimulator* cyanoSim)
{
	mCyanoSimulator = cyanoSim;
}

void GUIDebugger::drawCyanoHeadPos()
{
	if (!mCyanoSimulator) return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
	ImGui::Begin("HeadPos");

	//シアノのインデックスを設定
	ImGui::InputInt("Cyano Idx:", &mCyanoIdx);

	//インデックスが範囲内か判定
	const int maxIdx = mCyanoSimulator->mIndivisual_headPointIdx.size() - 1;
	if (mCyanoIdx > maxIdx || mCyanoIdx < 0) {
		ImGui::End();
		return;
	}

	//頭の位置を表示
	const XMFLOAT2 cyanoPos = mCyanoSimulator->mPoints_pos[mCyanoSimulator->mIndivisual_headPointIdx[mCyanoIdx]];
	std::string text = "x: " + std::to_string(cyanoPos.x) + " y: " + std::to_string(cyanoPos.y) + "\n";
	ImGui::Text(text.c_str());

	//角度を表示
	const float angle = mCyanoSimulator->mIndivisual_angle[mCyanoIdx];
	text = "angle: " + std::to_string(angle) + "\n";
	ImGui::Text(text.c_str());

	//セルインデックスを表示
	const int cellIdx = mCyanoSimulator->calcCellIdx(cyanoPos);
	text = "cellIdx: " + std::to_string(cellIdx) + "\n";
	ImGui::Text(text.c_str());

	ImGui::End();
	
}

static int selectedIndex = -1;          //選択中のオブジェクトのインデックス
static bool showEditorWindow = false;   //エディタウィンドウを表示するかどうか

//追加可能コンポーネント
static std::vector<std::string> componentList = {"SpriteComponent", "TextComponent", "MeshComponent"};
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
	//オブジェクトリストウィンドウ
	ImGui::Begin("Scene Hierarchy");

	//オブジェクトの表示
	for (int i = 0; i < objects.size(); i++) {
		//オブジェクト名が重複するとエラーが出るので、その対策にIDをpushする
		ImGui::PushID(i);
		//選択状態を判定してハイライトをつける
		bool isSelected = (selectedIndex == i);

		//オブジェクト名の取得
		std::string objectName = objects[i]->mName.empty() ? "(Unnamed)" : objects[i]->mName;

		if (ImGui::Selectable(objectName.c_str(), isSelected)) {
			selectedIndex = i;          //選択インデックスを更新
			showEditorWindow = true;    //エディタウィンドウのフラグを立てる
		}

		//pushしたIDをpop
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
		//追加可能なコンポーネントの型名を列挙し、選択できるようにする
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
		//選択した型名のコンポーネントを追加
		if (ImGui::Button("addComponent")) {
			const std::string& componentName = componentList[selectedComponentIndex];

			if (componentName == "SpriteComponent")	addSpriteComponent(object);
			else if (componentName == "TextComponent")	addTextComponent(object);
			else if (componentName == "MeshComponent") addMeshComponent(object);
		}

		//コンポーネントの編集
		componentEditer(object);

		//セーブ
		saveToObjectJsonButton(object);
		ImGui::End();
}

//JSON上でオブジェクト名をキーとし、各パラメータを保存しているため、オブジェクト名の重複を避けなければならない
void GUIDebugger::inputName(Object* obj, std::vector<class Object*>& objects)
{
	static std::string newName;
	ImGui::InputText("Name", &newName);
	ImGui::SameLine();
	static bool isError = false;
	if (ImGui::Button("apply")) {

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

			//シーンの初期オブジェクトに含まれている場合、シーン初期化用のJSONのオブジェクトIDも変える必要がある
			std::ifstream insceneFile("assets/data/sceneData.json");
			nlohmann::json sceneJson;
			insceneFile >> sceneJson;
			std::vector<std::string> objects = sceneJson[sceneName].get<std::vector<std::string>>();
			for (int i = 0; i < objects.size(); i++) {
				//初期オブジェクトに含まれる場合
				if (objects[i] == obj->mName) {
					objects[i] = newName;

					//JSONを変更
					sceneJson[sceneName] = std::move(objects);

					std::ofstream outsceneFile("assets/data/sceneData.json");
					outsceneFile << sceneJson.dump(4);
					break;
				}
			}

			//新たなオブジェクトIDに元データを移す
			j[sceneName][newName] = std::move(j[sceneName].value(obj->mName, R"({})"_json));
			//元オブジェクトIDをオブジェクトファイルから削除
			if (j[sceneName].contains(obj->mName)) j[sceneName].erase(obj->mName);
			obj->mName = newName;

			//オブジェクトファイルに保存
			std::ofstream outfile("assets/data/objectData.json");
			outfile << j.dump(4);
		}
	}

	//名前が重複していた場合
	if (isError) {
		ImGui::TextColored((ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), "the name is conflicted");
		ImGui::SameLine();
		
		//オブジェクトファイルに保存してあるオブジェクトIDを参照して複製できる
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

		//ラベルの型名を取得　コンポーネントのポインタが設定されていなかったらNONE
		std::string selectedComponentName;
		if (label.pComponent != nullptr) 
			selectedComponentName = label.pComponent->getComponentName();
		else
			selectedComponentName = "None";

		//選択可能なコンポーネント一覧を表示
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

		//オブジェクトファイルから削除
		std::ifstream infile("assets/data/objectData.json");
		nlohmann::json j;
		infile >> j;
		j[sceneName].erase(obj->mName);

		std::ofstream outfile("assets/data/objectData.json");
		outfile << j.dump(4);

		//シーンの初期に存在する場合、シーンファイルからも削除
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

//---各ウィンドウの定義用関数---
void GUIDebugger::objectDuplicateButton(Object* refObj, std::vector<Object*>& objects) {
	if (ImGui::Button("Duplicate")) {
		objectDuplicate(refObj->getName(), objects);
	}
}

void GUIDebugger::objectDuplicate(const std::string& refID, std::vector<Object*>& objects)
{

	auto& scene = mGame.getSceneManager().getCurrentScene();
	//現在、シーン中に同じ名前のオブジェクトが存在しているか判定
	std::ifstream infile("assets/data/objectData.json");
	nlohmann::json j;
	infile >> j;
	static bool dupFlag;
	dupFlag = true;
	static std::string newName;
	newName = refID;
	//オブジェクトの生成
	auto newObj = std::make_unique<Object>(scene, newName);

	//名前の重複を修正
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

		//名前が変更された場合、変更された名前が重複していないか確認する
		if (refID != newName) {
			//変更された名前がオブジェクトファイルにすでに登録されていたら、もう一度名前を変える
			if (j[scene.getName()].contains(newName) && j[scene.getName()][newName].size() != 0) {
				newName += "_copy";
				continue;
			}

			//jsonへ反映
			j[scene.getName()][newName] = j[scene.getName()][refID];
			newObj->setName(newName);
			//配列へ追加
			mGame.getSceneManager().getCurrentScene().addActor(std::move(newObj));

			//オブジェクトファイルに保存
			std::ofstream outfile("assets/data/objectData.json");
			outfile << j.dump(4);

			return;
		}

		dupFlag = false;
	}

	//名前が重複していなかった場合、ファイル操作をせずにシーンに追加
	mGame.getSceneManager().getCurrentScene().addActor(std::move(newObj));

}

//シーンの初期オブジェクトIDの保存
void GUIDebugger::saveToSceneJsonButton(std::vector<Object*>& objects)
{
	if (ImGui::Button("Save Scene")) {
		//シーンファイルを開く
		std::ifstream infile("assets/data/sceneData.json");
		nlohmann::json j;
		infile >> j;

		//シーンの名前を取得し、現在ヒエラルキーウィンドウに表示されているオブジェクトIDを保存する
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

//オブジェクトデータの保存
void GUIDebugger::saveToObjectJsonButton(Object* object)
{
	ImGui::Separator();
	if (ImGui::Button("Save Object")) {
		//オブジェクトファイルを開く
		std::ifstream infile("assets/data/objectData.json");
		nlohmann::json j;
		infile >> j;
		std::string sceneName = mGame.getSceneManager().getCurrentScene().getName();
		nlohmann::json& objJson = j[sceneName][object->getName()];

		//アクターデータをJSONに保存
		objJson["position"] = object->mPosition;
		objJson["rotation"] = object->mRotation;
		objJson["scale"] = object->mScale;

		//ラベルの保存
		std::vector<int> labelIDs;
		objJson["label"].clear();
		for (auto& [key, label] : object->mComponentLabels) {
			//ラベルが設定されていない場合無視
			if (label.pComponent == nullptr) {
				continue;
			}

			//ラベル付けされたコンポーネントの配列上のインデックスを取得
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

			//SpriteComponent
			if (componentName == "SpriteComponent") {
				saveSpriteComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			//TextComponent
			else if (componentName == "TextComponent") {
				saveTextComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
			else if (componentName == "MeshComponent") {
				saveMeshComponent(object->mComponents[i].get(), objJson["components"][componentIndex]);
				componentIndex++;
			}
		}

		//ファイルに出力
		std::ofstream outfile("assets/data/objectData.json");
		outfile << j.dump(4);
		infile.close();
		outfile.close();

		//アセットマネージャにもう一度オブジェクトファイルを読み込ませる
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
		if (componentName == "SpriteComponent")		 spriteComponentEditer(component);
		else if (componentName == "TextComponent")	 textComponentEditer(component);
		else if	(componentName == "MeshComponent")	 meshComponentEditer(component);

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
	//メッシュが設定されていない場合、MeshIDを入力できるようにする
	if (meshID == "NONE") {
		//メッシュIDの入力
		static std::string preMeshID;
		ImGui::InputText("##meshID", &preMeshID);
		ImGui::SameLine();
		if (ImGui::Button("Create")) {
			mesh->create(preMeshID);
		}

	}
	//メッシュが設定されている場合は何も変更できない
	else ImGui::Text(meshID.c_str());
	
}

void GUIDebugger::addMeshComponent(Object* object)
{
	auto mesh = std::make_unique<MeshComponent>(*object, mGraphic.getMeshBaseCBSuballocation());
	object->addComponent(std::move(mesh));
}

void GUIDebugger::saveMeshComponent(Component* component, nlohmann::json& objJson)
{
	//キャスト
	auto mesh = static_cast<MeshComponent*>(component);
	objJson["name"] = component->getComponentName();
	objJson["meshID"] = mesh->getMeshID();
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

	//テクスチャの適用
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
		ImGui::InputTextMultiline("Text", &text->mTextBuffer);

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
