#pragma once
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwrite.lib")
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3d11on12.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <dxgi1_6.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <vector>
#include "Definition.h"
#include "BIN_FILE12.h"
#include <winuser.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class PointLightComponent;
class SpotLightComponent;
class Game;

struct Vertex {
	XMFLOAT3 pos; //xyz座標
	XMFLOAT2 uv;  //uv座標
};

class Graphic
{
public:
	enum STATE {
		RENDER_3D,
		RENDER_2D,
		RENDER_DT,
	};

	Graphic(Game* game);
	~Graphic();

	void init();	
	HRESULT createBuf(UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer);
	HRESULT updateBuf(void* data, UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer);
	HRESULT mapBuf(void** mappedBuffer, ComPtr<ID3D12Resource>& buffer);
	void unmapBuf(ComPtr<ID3D12Resource>& buffer);
	UINT alignedSize(UINT size);
	HRESULT createShaderResource(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource);
	XMFLOAT2 createShaderResourceGetSize(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource);
	HRESULT createCbvTbvHeap(ComPtr<ID3D12DescriptorHeap>& cbvTbvHeap, UINT numDescriptors);
	void createVertexBufferView(ComPtr<ID3D12Resource>& vertexBuf, UINT sizeInBytes, UINT strideInBytes, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);
	void createIndexBufferView(ComPtr<ID3D12Resource>& indexBuf, UINT sizeInBytes, D3D12_INDEX_BUFFER_VIEW& indexBufferView);
	void createConstantBufferView(ComPtr<ID3D12Resource>& constantBuf, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void createConstantBufferView(int cbIndex, int cbSize, int heapIndex, int heapSize);
	void createBase3DBufferView(int heapIndex, int heapSize);
	void createShaderResourceView(ComPtr<ID3D12Resource>& shaderResource, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void createShaderResourceView(ID3D12Resource* shaderResource, int heapIndex);
	
	void clearColor(float r, float g, float b);
	void begin3DRender();
	void end3DRender();
	void moveToNextFrame();
	void prepareCommandList();

	bool quit();
	int msg_wparam();
	void closeEventHandle();
	void waitGPU();

	//Getter
	float getAspect();
	UINT getCbvTbvIncSize();
	ID3D12GraphicsCommandList* getCommandList();
	ID3D12CommandQueue* getCommandQueue();
	ID3D12CommandAllocator* getCommandAllocator();
	ID3D12Device* getDevice();
	float getClientWidth();
	float getClientHeight();
	ID3D11On12Device* getD3D11On12Device();
	ID3D11DeviceContext* getD3D11DeviceContext();
	ID2D1DeviceContext* getD2DDeviceContext();
	IDWriteFactory* getDWriteFactory();
	ID2D1Bitmap1* getD2DRenderTarget();
	UINT8* getConstantData();
	UINT8* getConstantData(int frame);
	D3D12_GPU_DESCRIPTOR_HANDLE getHeapHandle();
	int getBackBufIdx();

	//Setter
	void setRenderType(STATE state);

	//update
	void updateBase3DData(); //cameraの更新後に実行しなければいけない
	void updateViewProj(XMMATRIX& viewProj);
	void updatePointLight(const std::vector<PointLightComponent*>& lights);
	void updateSpotLight(const std::vector<SpotLightComponent*>& lights);
	void updateCameraPos(XMFLOAT4& cameraPos);
	void updateDamageFlashIntensity(float intensity);

private:
	HRESULT createDevice();
	HRESULT createCommand();
	HRESULT createFence();
	HRESULT createWindow();
	HRESULT createSwapChain();
	HRESULT createBbv();
	HRESULT createDSbuf();
	HRESULT createDSbv();
	HRESULT createPipeline();
	HRESULT createD2D();
	HRESULT createCbvAndHeap();

	

	//ウィンドウ
	LPCWSTR WindowTitle = L"DirectX12 Sample";
	const int ClientWidth = 1280;
	const int ClientHeight = 720;
	const int ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;
	const int ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;
	const float Aspect = static_cast<float>(ClientWidth) / ClientHeight;
#if 1
	DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
#else
	DWORD WindowStyle = WS_POPUP;
#endif

	HWND hWnd = nullptr;
	MSG Msg;
	static const int FrameCount = 2;

	//デバイス
	ComPtr<ID3D12Device> Device;
	ComPtr<ID2D1Device> mD2DDevice;
	//デバイスコンテキスト
	ComPtr<ID2D1DeviceContext> mD2DDeviceContext;
	//ファクトリー
	ComPtr<ID2D1Factory1> mD2DFactory;
	//コマンド
	ComPtr<ID3D12CommandAllocator> mCommandAllocator[FrameCount];
	ComPtr<ID3D12CommandAllocator> mLoadAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12GraphicsCommandList> mLoadList;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	//フェンス
	ComPtr<ID3D12Fence> mFence;
	HANDLE mFenceEvent;
	UINT64 mFenceValue;
	UINT64 mFenceValues[FrameCount] = {};
	//デバッグ
	HRESULT Hr;

	//リソース
	//バックバッファ
	ComPtr<IDXGISwapChain4> SwapChain;
	ComPtr<ID3D12Resource> BackBuffers[FrameCount];
	UINT BackBufIdx;
	ComPtr<ID3D12DescriptorHeap> BbvHeap; //BackBufferViewHeap
	UINT BbvHeapSize;
	float ClearColor[4];

	//デプスステンシルバッファ
	ComPtr<ID3D12Resource> DepthStencilBuf;
	ComPtr<ID3D12DescriptorHeap> DsvHeap; //DepthStencilBufView
	//パイプライン
	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12RootSignature> RootSignature2D;
	ComPtr<ID3D12RootSignature> RootSignatureDT;
	ComPtr<ID3D12PipelineState> PipelineState;
	ComPtr<ID3D12PipelineState> PipelineState2D;
	ComPtr<ID3D12PipelineState> PipelineStateDT;
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;

	//全3Dオブジェクト共通のデータ
	Base3DData Base3DData;

	//2D描画
	ComPtr<ID3D11On12Device> mD3D11On12Device;
	ComPtr<ID3D11DeviceContext> mD3D11DeviceContext;
	ComPtr<IDWriteFactory> mDWriteFactory;
	ComPtr<ID2D1Bitmap1> mD2DRenderTargets[FrameCount];
	ComPtr<ID3D11Resource> mWrappedBackBuffers[FrameCount];

	//共有して使用するヒープ、コンスタントバッファ
	ComPtr<ID3D12DescriptorHeap> mCbvTbvHeap;
	ComPtr<ID3D12Resource> mConstantBuf[FrameCount];
	UINT8* mConstantData[FrameCount];	//生データ

	Game* mGame;

};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
