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
    ID3D12DescriptorHeap*       Heap = nullptr;
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
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)            { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
    
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
        if (ImGui::SliderFloat("X", &text.mPosX, 0.0f, Graphic::ClientWidth)) {
            text.applyTextTexture();
        }
        if (ImGui::SliderFloat("Y", &text.mPosY, 0.0f, Graphic::ClientHeight)) {
			text.applyTextTexture();
        }
        if (ImGui::Button("Center")) {
            text.setPosition(text.mPosX - text.mTextWidth / 2.0f,text.mPosY);
        }
        if (ImGui::ColorEdit4("Color", text.mColorFloat.data())) {
			text.mTextColor = D2D1::ColorF(text.mColorFloat[0], text.mColorFloat[1], text.mColorFloat[2], text.mColorFloat[3]);
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
			j[text.mStructName]["x"] = text.mPosX;
			j[text.mStructName]["y"] = text.mPosY;
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

void GUIDebugger::drawObjectDebugGUI(std::vector<Object*>& objects)
{
    //シーン中のアクターを取得
    static int selectedIndex = -1;          //選択中のアクターインデックス
    static bool showEditorWindow = false;   //エディタウィンドウを表示するかどうか

    //ウィンドウの大きさ
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Appearing);
    //アクターリストウィンドウ
    ImGui::Begin("Scene Hierarchy");

    //追加可能コンポーネント
    static std::vector<std::string> componentList = {"MeshComponent", "PointLightComponent", "FireParticleComponent" };
    static int selectedComponentIndex = 0;

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
        mGame.getSceneManager().getCurrentScene().addDebugObject(object.get());
        mGame.getSceneManager().getCurrentScene().addActor(std::move(object));
    }

    //jsonへ保存
    if (ImGui::Button("Save to Json")) saveToJson(objects);
    ImGui::End();


    //エディタウィンドウ
    if (showEditorWindow && selectedIndex >= 0 && selectedIndex < objects.size()) {

        Object* object = objects[selectedIndex];

		//ウィンドウの大きさ
		ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
		ImGui::Begin("Editor", &showEditorWindow);

        ImGui::Text(("Editing: " + object->getClassName()).c_str());
        //複製ボタン
        ImGui::SameLine();
        objectDuplicateButton(object);
        //削除ボタン
        ImGui::SameLine();
        objectDeleteButton(object);
        ImGui::Separator();

        //オブジェクトの名前
        ImGui::InputText("Name", &object->mName);
		//位置
        ImGui::DragFloat3("Position", &object->mPosition.x, 0.1f);
        //回転
        ImGui::DragFloat3("Rotation", &object->mRotation.x, 0.1f);
        //スケール
		ImGui::DragFloat3("Scale", &object->mScale.x, 0.1f);

        //コンポーネント追加用ウィジェット
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
            //メッシュ
            if (componentName == "MeshComponent") {
                auto mesh = std::make_unique<MeshComponent>(*object);
                object->addComponent(std::move(mesh));
            }
            //点光源
            else if (componentName == "PointLightComponent") {
                auto light = std::make_unique<PointLightComponent>(*object);
                light->setActive(true);
                object->addComponent(std::move(light));
            }
            //炎パーティクル
            else if (componentName == "FireParticleComponent") {
                auto fire = std::make_unique<FireParticleComponent>(*object);
                object->addComponent(std::move(fire));
            }
        }

		//コンポーネントの設定
		for (int i = 0; i < object->mComponents.size(); i++) {
            //ハッシュのコンフリクトを防ぐため、IDをプッシュする
            ImGui::PushID(i);

			//PointLightComponent
			std::string componentName = object->mComponents[i]->getComponentName();
            if (componentName == "MeshComponent") {
                //コンポーネントの名前表示
				ImGui::Separator();
				ImGui::Text(componentName.c_str());
                
				//コンポーネントをキャスト
                MeshComponent* mesh = static_cast<MeshComponent*>(object->mComponents[i].get());

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

                //コンポーネントの除去
				if (ImGui::Button("RemoveComponent")) {
					object->removeComponent(mesh);
				}
			}
			if (componentName == "PointLightComponent") {

                //コンポーネントの名前表示
				ImGui::Separator();
				ImGui::Text(componentName.c_str());

				//コンポーネントをキャスト
				PointLightComponent* light = static_cast<PointLightComponent*>(object->mComponents[i].get());
                
                //オフセット位置
                ImGui::DragFloat4("LightOffsetPos", &light->mOffsetPos.x, 0.05f);
                //色
                ImGui::ColorEdit4("LightColor", &light->mColor.x);
                //強度
                ImGui::DragFloat("Intensity", &light->mIntensity);
                //範囲
                ImGui::DragFloat("Range", &light->mRange);

                //コンポーネントの除去
				if (ImGui::Button("RemoveComponent")) {
					object->removeComponent(light);
				}
            }

            //FireParticleComponent
            else if (componentName == "FireParticleComponent") {

                //コンポーネントの名前表示
				ImGui::Separator();
				ImGui::Text(componentName.c_str());

                //コンポーネントをキャスト
                FireParticleComponent* fire = static_cast<FireParticleComponent*>(object->mComponents[i].get());

                //エミット位置
                ImGui::DragFloat3("ParticleEmitPos", &fire->mEmitterPosition.x);

                //コンポーネントの除去
				if (ImGui::Button("RemoveComponent")) {
					object->removeComponent(fire);
				}
            }

            //IDのポップ
            ImGui::PopID();
        }


        ImGui::End();
    }
}

void GUIDebugger::updateCameraPos(XMFLOAT3& position)
{
    mCameraPos = position;
}

void GUIDebugger::objectDeleteButton(Object* obj)
{
    if (ImGui::Button("Delete")) {

        mGame.getSceneManager().getCurrentScene().removeDebugObject(obj);
        obj->mState = Actor::State::Dead;

    }
}

//各ウィンドウの定義用関数
void GUIDebugger::objectDuplicateButton(Object* refObj) {
    if (ImGui::Button("Duplicate")) {

        //インスタンスの作成
        auto newObj = std::make_unique<Object>(mGame.getSceneManager().getCurrentScene(), refObj->mName);
        newObj->setPosition(refObj->mPosition);
        newObj->setRotation(refObj->mRotation);
        newObj->setScale(refObj->mScale);

        //コンポーネントの取得
        for (int i = 0; i < refObj->mComponents.size(); i++) {

            std::string componentName = refObj->mComponents[i]->getComponentName();
            //メッシュ
            if (componentName == "MeshComponent") {
                //複製元のコンポーネントをキャスト
                auto refMesh = static_cast<MeshComponent*>(refObj->mComponents[i].get());

                auto mesh = std::make_unique<MeshComponent>(*newObj);
                mesh->create(refMesh->getMeshID());
                newObj->addComponent(std::move(mesh));
            }
            //点光源
            if (componentName == "PointLightComponent") {
                //複製元のコンポーネントをキャスト
                auto refLight = static_cast<PointLightComponent*>(refObj->mComponents[i].get());
                //複製
                auto light = std::make_unique<PointLightComponent>(*newObj);
                light->setOffsetPos(refLight->mOffsetPos);
                light->setColor(refLight->mColor);
                light->setIntensity(refLight->mIntensity);
                light->setRange(refLight->mRange);
                light->setActive(true);
                newObj->addComponent(std::move(light));
            }
            //炎パーティクル
            else if (componentName == "FireParticleComponent") {
                //複製元のコンポーネントをキャスト
                auto refFire = static_cast<FireParticleComponent*>(refObj->mComponents[i].get());
                //複製
                auto fire = std::make_unique<FireParticleComponent>(*newObj);
                fire->setEmitterPosition(refFire->mEmitterPosition);
                newObj->addComponent(std::move(fire));
            }
        }

        //配列へ追加
        mGame.getSceneManager().getCurrentScene().addDebugObject(newObj.get());
        mGame.getSceneManager().getCurrentScene().addActor(std::move(newObj));
    }
}

void GUIDebugger::saveToJson(std::vector<Object*>& objects)
{
	std::ifstream infile("assets/data/mapObjectData.json");
	nlohmann::json j;
	infile >> j;

	for (int jsonIndex = 0; jsonIndex < objects.size(); jsonIndex++) {
        Object* object = objects[jsonIndex];
		//アクターデータ
		j[jsonIndex]["name"] = object->mName;
		j[jsonIndex]["position"] = object->mPosition;
		j[jsonIndex]["rotation"] = object->mRotation;
		j[jsonIndex]["scale"] = object->mScale;

		//コンポーネントデータ
		int componentIndex = 0;
		for (int i = 0; i < object->mComponents.size(); i++) {
			std::string componentName = object->mComponents[i]->getComponentName();

			//MeshComponent
			if (componentName == "MeshComponent") {
				//キャスト
				auto mesh = static_cast<MeshComponent*>(object->mComponents[i].get());
				j[jsonIndex]["components"][componentIndex]["name"] = componentName;
				j[jsonIndex]["components"][componentIndex]["meshID"] = mesh->getMeshID();

                componentIndex++;
				continue;
			}

			//PointLightComponent
			else if (componentName == "PointLightComponent") {
				//キャスト
				auto light = static_cast<PointLightComponent*>(object->mComponents[i].get());

				//データの格納
				j[jsonIndex]["components"][componentIndex]["name"] = componentName;
				j[jsonIndex]["components"][componentIndex]["lightOffsetPos"] = light->mOffsetPos;
				j[jsonIndex]["components"][componentIndex]["lightColor"] = light->mColor;
				j[jsonIndex]["components"][componentIndex]["intensity"] = light->mIntensity;
				j[jsonIndex]["components"][componentIndex]["range"] = light->mRange;
                componentIndex++;
				continue;
			}

			//FireParticleComponent
			else if (componentName == "FireParticleComponent") {
				auto fire = static_cast<FireParticleComponent*>(object->mComponents[i].get());

				//データの格納
				j[jsonIndex]["components"][componentIndex]["name"] = componentName;
				j[jsonIndex]["components"][componentIndex]["particleEmitPos"] = fire->mEmitterPosition;
                componentIndex++;
				continue;                  			}
		}
	}
	std::ofstream outfile("assets/data/mapObjectData.json");
	outfile << j.dump(4); // 4はインデントスペースの数			
}
