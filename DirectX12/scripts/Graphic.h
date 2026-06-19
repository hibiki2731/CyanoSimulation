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

//前方宣言
class Game;

//GraphicクラスでDirectX12の初期化、リソース管理、描画処理を行う
/*
-----バッファ等のルール-----
・バーテックスバッファ（頂点情報を格納するバッファ）
　AssetManagerクラスで作成、管理する。同時にビューも作成
  AssetManagerクラスが1種類のメッシュに対し、1バッファに制御する
  AssetManagerクラスからMeshIDをキーとして、ビューを取得できる。

・インデックスバッファ（頂点の描画順を格納するバッファ)
  基本使わない

・コンスタントバッファ（シェーダーに渡す定数を格納するバッファ）
  CPUとGPUの非同期処理のため、フレーム数分の大きなバッファを作成し、フレームごとに切り替えて使用する
  オブジェクトごとにコンスタントバッファ内のアドレスをずらして使用する。
  アドレスはAssetManagerクラスで管理

・シェーダーリソース（テクスチャなどのリソース）
  AssetManagerクラスで作成、管理する。MeshIDをキーとして、ビューを取得できる。
  1種類のテクスチャにつき1リソースに制御する

・ディスクリプタヒープ（ビューを格納するヒープ）
  一つの大きなヒープを作成
  オブジェクトごとに、ヒープ内のアドレスをずらして使用する。アドレスはAssetManagerクラスで管理
  CPUとGPUの非同期処理のため、フレーム数分同じビューを作成する必要がある。
  フレーム毎のビューは同じ構造で連続して配置する必要がある。
  例えばコンスタントバッファ1(CB1)、コンスタントバッファ2(CB2)、シェーダーリソース1(SR1)の順でビューを2フレーム分作成する場合、
  CB1、CB2、SR1、CB1、CB2、SR1の順でビューを配置する必要がある。

*/

class Graphic
{
public:
	//レンダリングの種類　パイプラインステートやルートシグネチャを切り替えるために使用
	enum STATE {
		RENDER_3D,	//3Dオブジェクトの描画
		RENDER_2D,	//UIなどの2D描画
		RENDER_DT,	//ダメージテキストの描画
		RENDER_FP	//炎パーティクルの描画
	};

	Graphic(Game& game);
	~Graphic();

	void init();				//初期化
	//---ディスクリプタヒープの作成---
	HRESULT createCbvTbvHeap(ComPtr<ID3D12DescriptorHeap>& cbvTbvHeap, UINT numDescriptors);	//CBVとSRV用のディスクリプタヒープの作成
	//---リソースの作成、更新、マッピング---
	HRESULT  createBuf(UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer);				//バッファの作成
	HRESULT  updateBuf(void* data, UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer);	//バッファの更新
	HRESULT  mapBuf(void** mappedBuffer, ComPtr<ID3D12Resource>& buffer);				//バッファのマッピング
	void     unmapBuf(ComPtr<ID3D12Resource>& buffer);									//バッファのマッピング解除
	HRESULT  createShaderResource(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource);	//テクスチャの作成
	XMFLOAT2 createShaderResourceGetSize(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource);	//テクスチャの作成とサイズの取得
	//---バッファビューの作成---
	void createVertexBufferView(ComPtr<ID3D12Resource>& vertexBuf, UINT sizeInBytes, UINT strideInBytes, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);	//頂点バッファビューの作成
	void createIndexBufferView(ComPtr<ID3D12Resource>& indexBuf, UINT sizeInBytes, D3D12_INDEX_BUFFER_VIEW& indexBufferView);	//インデックスバッファビューの作成
	void createConstantBufferView(int cbIndex, int cbSize, int heapIndex, int heapSize);	//コンスタントバッファビューの作成(2フレーム分コンスタントバッファ内の領域を確保するため、heapSize間隔で2つビューを作成)
	void createBase3DBufferView(int heapIndex, int heapSize);								//3Dオブジェクトが共通して使うコンスタントバッファビューの作成(2フレーム分)
	void createShaderResourceView(ID3D12Resource* shaderResource, int heapIndex);			//シェーダーリソースビューの作成
	

	//---描画処理---
	void beginRender();			//レンダリング開始前の処理(レンダーターゲットの設定、画面クリアなど)
	void endRender();			//レンダリング終了後の処理(コマンドリストのクローズ、GPUへのコマンドの送信など)
	void moveToNextFrame();		//フレームの切り替え。1フレーム前のGPUの処理が終わっていなかったら、待機する。

	//---その他---
	void clearColor(float r, float g, float b);			//画面クリアの色の設定
	bool quit();										//ウィンドウの×ボタンが押されたか
	int msg_wparam();									//ウィンドウメッセージのwparamを取得
	void closeEventHandle();							//ウィンドウの×ボタンが押されたときの処理
	void waitGPU();										//GPUの処理が終わるまで待機する
	void delayRelease(ComPtr<IUnknown>& resource);		//リソースの開放を遅らせる。GPUがリソースを使用している可能性がある場合に、すぐに開放せず、次のフレームで開放する。
	static UINT alignedSize(UINT size);					//引数のサイズを256バイトアライメントにする

	//getter
	HWND getWindowHandle();
	float getAspect();
	UINT getCbvTbvIncSize();
	ID3D12GraphicsCommandList* getCommandList();
	ID3D12CommandQueue* getCommandQueue();
	ID3D12CommandAllocator* getCommandAllocator();
	ID3D12Device* getDevice();
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
	void setRenderType(STATE state);	//描画するオブジェクトの種類に応じて、パイプラインステートやルートシグネチャを切り替える

	//ウィンドウパラメータ
	static constexpr LPCWSTR WindowTitle = L"The Dungeon";
	static constexpr int	 ClientWidth = 1280;
	static constexpr int	 ClientHeight = 720;
	static constexpr float	 Aspect = static_cast<float>(ClientWidth) / ClientHeight;
	static constexpr int	 FrameCount = 2;

#ifdef _DEBUG
	void setShareDescriptor();
#endif
private:
	//初期化関数
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

	//---ウィンドウ---
	const int ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;
	const int ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;
	HWND hWnd = nullptr;		//ウィンドウハンドル
	MSG Msg;					//ウィンドウメッセージ
#if 1
	DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
#else
	DWORD WindowStyle = WS_POPUP;
#endif

	/*
	本プログラムでは、DirectX12とDirect2Dを両方使用している。
	DirectX12は3Dオブジェクトやスプライトの描画に使用し、Direct2Dはテキスト描画に使用する。
	Direct2DはDirectX11のリソースに対してのみ描画できるため、DirectX12のリソースをDirectX11On12でラップして、Direct2Dで描画できるようにしている。
	*/
	//---DirectX12---
	//デバイス
	ComPtr<ID3D12Device> Device;
	//コマンド
	ComPtr<ID3D12CommandAllocator> mCommandAllocator[FrameCount];	//フレーム数分用意
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12CommandAllocator> mLoadAllocator;					//リソースの読み込みに使用するコマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> mLoadList;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	//フェンス
	ComPtr<ID3D12Fence> mFence;				//GPUの処理完了をチェックするフェンス
	HANDLE mFenceEvent;						//フェンスのシグナルを待機するためのイベントハンドル
	UINT64 mFenceValue;						//フェンスの値。毎フレーム+1していき、GPUの処理がどこまで進んでいるかを管理する。
	UINT64 mFenceValues[FrameCount] = {};	//フレームごとのフェンスの値。非同期処理のため、前フレームのGPUの処理が終わっているかを確認するために使用する。

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
	ComPtr<ID3D12RootSignature> RootSignature2D;
	ComPtr<ID3D12PipelineState> PipelineState2D;
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;
	//共有して使用するヒープ、コンスタントバッファ
	ComPtr<ID3D12DescriptorHeap> mCbvTbvHeap;
	ComPtr<ID3D12Resource> mConstantBuf[FrameCount];
	UINT8* mConstantData[FrameCount];	//生データ

	//Direct2D
	ComPtr<ID3D11On12Device> mD3D11On12Device;
	ComPtr<ID3D11DeviceContext> mD3D11DeviceContext;
	ComPtr<IDWriteFactory> mDWriteFactory;
	ComPtr<ID2D1Bitmap1> mD2DRenderTargets[FrameCount];
	ComPtr<ID3D11Resource> mWrappedBackBuffers[FrameCount];
	//---デバイス---
	ComPtr<ID2D1Device> mD2DDevice;	//D2Dの描画に使用
	//---デバイスコンテキスト---
	ComPtr<ID2D1DeviceContext> mD2DDeviceContext;	//D2Dの描画に使用
	//---ファクトリー---
	ComPtr<ID2D1Factory1> mD2DFactory;	//D2Dのデバイスを作成するために使用

	//遅延削除用のごみ箱
	std::vector<ComPtr<IUnknown>> mTrashQueue[FrameCount];

	//参照
	Game& mGame;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
