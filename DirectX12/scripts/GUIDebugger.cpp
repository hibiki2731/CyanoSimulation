#include "GUIDebugger.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "fstream"
#include "json.hpp"
#include "Imgui/imgui_stdlib.h"
#include "MyUtility.h"

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

GUIDebugger::GUIDebugger(Graphic& graphic)
	: mGraphic(graphic)
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
	HRESULT hr = graphic.getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mSrvHeap.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));
	g_pd3dSrvDescHeapAlloc.Create(graphic.getDevice(), mSrvHeap.Get());


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
	ImGui_ImplWin32_Init(graphic.getWindowHandle());

	//DirectX12バックエンドの初期化
    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = graphic.getDevice();
    init_info.CommandQueue = graphic.getCommandQueue();
    init_info.NumFramesInFlight = Graphic::FrameCount;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    init_info.SrvDescriptorHeap = mSrvHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)            { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
    
	ImGui_ImplDX12_Init(&init_info);
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
            
			std::ofstream outfile("assets/data/textData.json");
			outfile << j.dump(4); // 4はインデントスペースの数			
        }

	}
	ImGui::End();
}

