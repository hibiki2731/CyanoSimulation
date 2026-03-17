#include "Graphic.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"
#include "Game.h"

Graphic::Graphic(Game* game)
{
	ClearColor[0] = 1.0f;
	ClearColor[1] = 0.4f;
	ClearColor[2] = 0.45f;
	ClearColor[3] = 1.0f;

	BackBufIdx = 0;
	mGame = game;

	Base3DData.playerFlashColor = XMFLOAT3(1.0f, 0.0f, 0.0f);
	Base3DData.playerFlashIntensity = 0.0f;
}

Graphic::~Graphic()
{
	for (int i = 0; i < FrameCount; i++) mConstantBuf[i]->Unmap(0, nullptr);
	waitGPU();
}


void Graphic::init() {
	HRESULT hr;
	//ウィンドウの作成
	hr = createWindow();
	assert(SUCCEEDED(hr));
	//デバイスの作成
	hr = createDevice();
	assert(SUCCEEDED(hr));
	//コマンド作成
	hr = createCommand();
	assert(SUCCEEDED(hr));
	//フェンス 処理完了のチェック
	hr = createFence();
	assert(SUCCEEDED(hr));

	//スワップチェイン、バックバッファの作成
	hr = createSwapChain();
	assert(SUCCEEDED(hr));
	//バックバッファビューの作成
	hr = createBbv();
	assert(SUCCEEDED(hr));
	//デプスステンシルバッファの作成
	hr = createDSbuf();
	assert(SUCCEEDED(hr));
	//デプスステンシルビューの作成
	hr = createDSbv();
	assert(SUCCEEDED(hr));

	//パイプラインステートの作成
	hr = createPipeline();
	assert(SUCCEEDED(hr));

	//共有用ディスクリプタヒープ、コンスタントバッファの作成
	hr = createCbvAndHeap();
	assert(SUCCEEDED(hr));

	//D2Dの初期化
	hr = createD2D();
	assert(SUCCEEDED(hr));

	ShowWindow(hWnd, SW_SHOW);
}

HRESULT Graphic::createDevice() {
	{
#ifdef _DEBUG
		//デバッグレイヤーをオンに
		ComPtr<ID3D12Debug> debug;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		assert(SUCCEEDED(hr));
		debug->EnableDebugLayer();
#endif
	}

	HRESULT hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(Device.GetAddressOf())
	);

#ifdef _DEBUG
	//誤検知エラーを無視するフィルタの作成
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(Device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_REFLECTSHAREDPROPERTIES_INVALIDOBJECT
		};

		D3D12_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;

		//フィルタを適用
		infoQueue->PushStorageFilter(&filter);
	}

#endif
	return hr;
}

HRESULT Graphic::createCommand() {
	//コマンドアロケータ作成
	for (int i = 0; i < FrameCount; i++) {
		HRESULT hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(mCommandAllocator[i].GetAddressOf()));
		if (FAILED(hr))	return hr;
		
	}
	HRESULT hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mLoadAllocator.GetAddressOf()));
	assert(SUCCEEDED(hr));

	//コマンドリスト作成
	hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mCommandAllocator[0].Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())
	);
	assert(SUCCEEDED(hr));
	hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		mLoadAllocator.Get(), nullptr, IID_PPV_ARGS(mLoadList.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	//コマンドキュー作成
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//GPUタイムアウトが有効
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; //直接コマンドキュー
	hr = Device->CreateCommandQueue(&desc, IID_PPV_ARGS(mCommandQueue.GetAddressOf()));
	return hr;
}

HRESULT Graphic::createFence() {
	//GPUの処理完了をチェックするフェンスを作る
	HRESULT hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mFence.GetAddressOf()));
	if (FAILED(hr))
	{
		return hr;
	}
	mFenceValue = 1;
	mFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	return hr;
}

HRESULT Graphic::createWindow() {
	//ウィンドウクラスの登録
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszClassName = L"DirectX12WindowClass";
	RegisterClassEx(&windowClass);
	//表示位置、ウィンドウの大きさ調節
	RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
	AdjustWindowRect(&windowRect, WindowStyle, FALSE);
	int windowPosX = ClientPosX + windowRect.left;
	int windowPosY = ClientPosY + windowRect.top;
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;
	//ウィンドウの生成
	hWnd = CreateWindow(
		windowClass.lpszClassName,
		WindowTitle,
		WindowStyle,
		windowPosX,
		windowPosY,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		windowClass.hInstance,
		nullptr
	);
	//ShowWindow(hWnd, SW_SHOW);

	return S_OK;
}

HRESULT Graphic::createSwapChain() {
	//DXGIファクトリーの生成、スワップチェイン作成(バックバッファもここで作成)
	ComPtr<IDXGIFactory4> dxgiFactory;
	HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	//スワップチェインの作成

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferCount = 2; //ダブルバッファ
	desc.Width = ClientWidth;
	desc.Height = ClientHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色の書式
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //フリップ後は保証しない（その代わり早い）
	desc.SampleDesc.Count = 1;
	ComPtr<IDXGISwapChain1> swapChain1;
	hr = dxgiFactory->CreateSwapChainForHwnd(
		mCommandQueue.Get(), hWnd, &desc, nullptr, nullptr, swapChain1.GetAddressOf()
	);
	if (FAILED(hr)) {
		return hr;
	}

	//IDGISwapChain4のインターフェイスがサポートしているか確認、取得
	hr = swapChain1->QueryInterface(IID_PPV_ARGS(SwapChain.GetAddressOf()));
	return hr;
}

HRESULT Graphic::createBbv() {

	//デスクリプターヒープ（ビューを記憶する場所）の作成
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 2; //バックバッファの数
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビュー
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //シェーダからアクセスしない
	HRESULT hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(BbvHeap.GetAddressOf()));
	assert(SUCCEEDED(hr));
	//バックバッファビューの作成
	
	//ポインタにヒープのアドレスを入れる
	D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap = BbvHeap->GetCPUDescriptorHandleForHeapStart();
	//ビューのサイズ
	BbvHeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT idx = 0; idx < 2; idx++) {
		//スワップチェイン内のバックバッファを取り出す
		hr = SwapChain->GetBuffer(idx, IID_PPV_ARGS(BackBuffers[idx].GetAddressOf()));
		assert(SUCCEEDED(hr));
		//バックバッファのビューを作成
		hBbvHeap.ptr += idx * BbvHeapSize;
		Device->CreateRenderTargetView(BackBuffers[idx].Get(), nullptr, hBbvHeap);
	}
	return hr;
}

HRESULT Graphic::createDSbuf() {
	//デプスステンシルバッファを作る
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT; //DEFAULTだからあとはUNKNOWN
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = ClientWidth; //レンダーターゲットと同じ
	desc.Height = ClientHeight;
	desc.DepthOrArraySize = 1; //二次元のテクスチャデータとして　
	desc.Format = DXGI_FORMAT_D32_FLOAT; //深度書き込み用フォーマット
	desc.SampleDesc.Count = 1; //サンプルは1ピクセルあたり1つ
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; //深度ステンシルとして使用
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = 1;
	//デプスステンシルバッファをクリアする値
	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f; //深さ1(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32bit深度値としてクリア
	//デプスステンシルバッファを作る
	HRESULT hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
		&depthClearValue,
		IID_PPV_ARGS(DepthStencilBuf.GetAddressOf())
	);
	return hr;
}

HRESULT Graphic::createDSbv() {
	//デプスステンシルバッファビューの入れ物であるディスクリプタヒープを作る
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 1; //深度ビュー1るのみ
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; //デプスステンシルビューのディスクリプタヒープ
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));
	}
	//デプスステンシルバッファビューをディスクリプタヒープに作る
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_D32_FLOAT; //デプス値に32bit
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2Dテクスチャ
		desc.Flags = D3D12_DSV_FLAG_NONE; //フラグなし
		D3D12_CPU_DESCRIPTOR_HANDLE hDsvHeap = DsvHeap->GetCPUDescriptorHandleForHeapStart();
		Device->CreateDepthStencilView(DepthStencilBuf.Get(), &desc, hDsvHeap);
	}
	return S_OK;
}

HRESULT Graphic::createPipeline()
{
	//3D用パイプラインステート
	{
		//ルートシグネチャ
		//ディスクリプタレンジ、ディスクリプタヒープとシェーダを紐づける役割を持つ
		D3D12_DESCRIPTOR_RANGE range[2] = {};
		UINT b0 = 0;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; //定数バッファビュー
		range[0].BaseShaderRegister = b0;
		range[0].NumDescriptors = 3; //b0,b1,b2
		range[0].RegisterSpace = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		UINT t0 = 0;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //シェーダリソースビュー
		range[1].BaseShaderRegister = t0;
		range[1].NumDescriptors = 1; //t0だけ
		range[1].RegisterSpace = 0;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		//ルートパラメタをディスクリプタテーブルとして使用
		//rangeの入れ物
		D3D12_ROOT_PARAMETER rootParam[1] = {};
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[0].DescriptorTable.pDescriptorRanges = range;
		rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(range);
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //全てのシェーダから見える

		//サンプラの記述
		D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; //補完しない
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //U方向は繰り返し
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //V方向は繰り返し
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //W方向は繰り返し
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //ボーダーの時は黒
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX; //ミップマップ最大
		samplerDesc[0].MinLOD = 0.0f; //ミップマップ最小
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //オーバーサンプリングの際リサンプリングしない
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //ピクセルシェーダからのみ見える

		//ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //入力アセンブラの入力レイアウトを許可
		desc.pParameters = rootParam;
		desc.NumParameters = _countof(rootParam);
		desc.pStaticSamplers = samplerDesc;  //サンプラーの先頭アドレス
		desc.NumStaticSamplers = _countof(samplerDesc); //サンプラーの数

		//ルートシグネチャをシリアライズ(コンパイルするようなもの)
		ComPtr<ID3DBlob> blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), nullptr);
		assert(SUCCEEDED(hr));

		//ルートシグネチャの作成
		hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(RootSignature.GetAddressOf()));
		assert(SUCCEEDED(hr));
		
		
		//シェーダの読み込み
		BIN_FILE12 vs("assets\\VertexShader.cso");
		assert(vs.succeeded());
		BIN_FILE12 ps("assets\\PixelShader.cso");
		assert(ps.succeeded());

		//各種記述
		UINT slot0 = 0;
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    slot0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FrontCounterClockwise = true; //反時計回り
		rasterDesc.CullMode = D3D12_CULL_MODE_BACK; //裏面描画するか？
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = true;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //書き込み許可
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さいほうが手前
		depthStencilDesc.StencilEnable = FALSE; //ステンシルしない

		//ここまでの記述をまとめてパイプラインステートオブジェクトを作成
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
		pipelineDesc.pRootSignature = RootSignature.Get();
		pipelineDesc.VS = { vs.code(), vs.size() };
		pipelineDesc.PS = { ps.code(), ps.size() };
		pipelineDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		pipelineDesc.RasterizerState = rasterDesc;
		pipelineDesc.BlendState = blendDesc;
		pipelineDesc.DepthStencilState = depthStencilDesc;
		pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineDesc.SampleMask = UINT_MAX;
		pipelineDesc.SampleDesc.Count = 1;
		pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineDesc.NumRenderTargets = 1;
		pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		hr = Device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(PipelineState.GetAddressOf()));
		assert(SUCCEEDED(hr));
		
	} {}

	//2D用パイプラインステート
	{
		//2D用ルートシグネチャ
		//ディスクリプタレンジ、ディスクリプタヒープとシェーダを紐づける役割を持つ
		D3D12_DESCRIPTOR_RANGE range[2] = {};
		UINT b0 = 0;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; //定数バッファビュー
		range[0].BaseShaderRegister = b0;
		range[0].NumDescriptors = 1;
		range[0].RegisterSpace = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		UINT t0 = 0;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //シェーダリソースビュー
		range[1].BaseShaderRegister = t0;
		range[1].NumDescriptors = 1; //t0だけ
		range[1].RegisterSpace = 0;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		D3D12_ROOT_PARAMETER rootParam[2] = {};
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[0].DescriptorTable.pDescriptorRanges = &range[0];
		rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //全てのシェーダから見える
		rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[1].DescriptorTable.pDescriptorRanges = &range[1];
		rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //全てのシェーダから見える

		//サンプラの記述
		D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; //補完しない
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //U方向は繰り返し
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //V方向は繰り返し
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //W方向は繰り返し
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //ボーダーの時は黒
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX; //ミップマップ最大
		samplerDesc[0].MinLOD = 0.0f; //ミップマップ最小
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //オーバーサンプリングの際リサンプリングしない
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //ピクセルシェーダからのみ見える

		//ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //入力アセンブラの入力レイアウトを許可
		desc.pParameters = rootParam;
		desc.NumParameters = _countof(rootParam);
		desc.pStaticSamplers = samplerDesc;  //サンプラーの先頭アドレス
		desc.NumStaticSamplers = _countof(samplerDesc); //サンプラーの数

		//ルートシグネチャをシリアライズ(コンパイルするようなもの)
		ComPtr<ID3DBlob> blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), nullptr);
		assert(SUCCEEDED(hr));

		//ルートシグネチャの作成
		hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(RootSignature2D.GetAddressOf()));
		assert(SUCCEEDED(hr));

		{
			//シェーダの読み込み
			BIN_FILE12 vs2D("assets\\2DVertexShader.cso");
			assert(vs2D.succeeded());
			BIN_FILE12 ps2D("assets\\2DPixelShader.cso");
			assert(ps2D.succeeded());

			//各種記述
			UINT slot0 = 0;
			D3D12_INPUT_ELEMENT_DESC inputElementDescs2D[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, slot0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			};

			D3D12_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.FrontCounterClockwise = true; //反時計回り
			rasterDesc.CullMode = D3D12_CULL_MODE_BACK; //裏面描画するか？
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterDesc.DepthClipEnable = TRUE;
			rasterDesc.MultisampleEnable = FALSE;
			rasterDesc.AntialiasedLineEnable = FALSE;
			rasterDesc.ForcedSampleCount = 0;
			rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			D3D12_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = true;
			blendDesc.IndependentBlendEnable = FALSE;
			blendDesc.RenderTarget[0].BlendEnable = false;
			blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //書き込み許可
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さいほうが手前
			depthStencilDesc.StencilEnable = FALSE; //ステンシルしない
			//ここまでの記述をまとめてパイプラインステートオブジェクトを作成
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc2D = {};
			pipelineDesc2D.pRootSignature = RootSignature2D.Get();
			pipelineDesc2D.VS = { vs2D.code(), vs2D.size() };
			pipelineDesc2D.PS = { ps2D.code(), ps2D.size() };
			pipelineDesc2D.InputLayout = { inputElementDescs2D, _countof(inputElementDescs2D) };
			pipelineDesc2D.RasterizerState = rasterDesc;
			pipelineDesc2D.BlendState = blendDesc;
			pipelineDesc2D.DepthStencilState = depthStencilDesc;
			pipelineDesc2D.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			pipelineDesc2D.SampleMask = UINT_MAX;
			pipelineDesc2D.SampleDesc.Count = 1;
			pipelineDesc2D.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineDesc2D.NumRenderTargets = 1;
			pipelineDesc2D.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			HRESULT hr = Device->CreateGraphicsPipelineState(&pipelineDesc2D, IID_PPV_ARGS(PipelineState2D.GetAddressOf()));
			assert(SUCCEEDED(hr));
		}
	} {}


	//ダメージエフェクト用パイプラインステート
	{
		//ダメージエフェクト用ルートシグネチャ
		//ディスクリプタレンジ、ディスクリプタヒープとシェーダを紐づける役割を持つ
		D3D12_DESCRIPTOR_RANGE range[2] = {};
		UINT b0 = 0;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; //定数バッファビュー
		range[0].BaseShaderRegister = b0;
		range[0].NumDescriptors = 1;
		range[0].RegisterSpace = 0;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		UINT t0 = 0;
		range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //シェーダリソースビュー
		range[1].BaseShaderRegister = t0;
		range[1].NumDescriptors = 1; //t0だけ
		range[1].RegisterSpace = 0;
		range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //自動計算

		D3D12_ROOT_PARAMETER rootParam[1] = {};
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam[0].DescriptorTable.pDescriptorRanges = range;
		rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(range);
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //全てのシェーダから見える

		//サンプラの記述
		D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
		samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; //補完しない
		samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //U方向は繰り返し
		samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //V方向は繰り返し
		samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //W方向は繰り返し
		samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //ボーダーの時は黒
		samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX; //ミップマップ最大
		samplerDesc[0].MinLOD = 0.0f; //ミップマップ最小
		samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //オーバーサンプリングの際リサンプリングしない
		samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //ピクセルシェーダからのみ見える

		//ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //入力アセンブラの入力レイアウトを許可
		desc.pParameters = rootParam;
		desc.NumParameters = _countof(rootParam);
		desc.pStaticSamplers = samplerDesc;  //サンプラーの先頭アドレス
		desc.NumStaticSamplers = _countof(samplerDesc); //サンプラーの数

		//ルートシグネチャをシリアライズ(コンパイルするようなもの)
		ComPtr<ID3DBlob> blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), nullptr);
		assert(SUCCEEDED(hr));

		//ルートシグネチャの作成
		hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(RootSignatureDT.GetAddressOf()));
		assert(SUCCEEDED(hr));

		{
			//シェーダの読み込み
			BIN_FILE12 vsDT("assets\\DTVertexShader.cso");
			assert(vsDT.succeeded());
			BIN_FILE12 gsDT("assets\\DTGeometryShader.cso");
			assert(gsDT.succeeded());
			BIN_FILE12 psDT("assets\\DTPixelShader.cso");
			assert(psDT.succeeded());

			//各種記述
			UINT slot0 = 0; //IAsetVertexBuffersでバッファをセットするスロット番号
			//何度も更新するため、インスタンスデータを使用する。D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATAにする。最後の1は、1インスタンスごとにデータが更新されることを意味する。
			D3D12_INPUT_ELEMENT_DESC inputElementDescsDT[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, 0,  D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
				{"SIZE", 0, DXGI_FORMAT_R32_FLOAT, slot0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
				{"DIGIT", 0, DXGI_FORMAT_R32_FLOAT, slot0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
				{"ALPHA", 0, DXGI_FORMAT_R32_FLOAT, slot0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
			};

			D3D12_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.FrontCounterClockwise = true; //反時計回り
			rasterDesc.CullMode = D3D12_CULL_MODE_NONE; //裏面描画するか？
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterDesc.DepthClipEnable = TRUE;
			rasterDesc.MultisampleEnable = FALSE;
			rasterDesc.AntialiasedLineEnable = FALSE;
			rasterDesc.ForcedSampleCount = 0;
			rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			D3D12_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = true;
			blendDesc.IndependentBlendEnable = FALSE;
			blendDesc.RenderTarget[0].BlendEnable = false;
			blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //書き込み許可
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さいほうが手前
			depthStencilDesc.StencilEnable = FALSE; //ステンシルしない

			//ここまでの記述をまとめてパイプラインステートオブジェクトを作成
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDescDT = {};
			pipelineDescDT.pRootSignature = RootSignatureDT.Get();
			pipelineDescDT.VS = { vsDT.code(), vsDT.size() };
			pipelineDescDT.GS = { gsDT.code(), gsDT.size() };
			pipelineDescDT.PS = { psDT.code(), psDT.size() };
			pipelineDescDT.InputLayout = { inputElementDescsDT, _countof(inputElementDescsDT) };
			pipelineDescDT.RasterizerState = rasterDesc;
			pipelineDescDT.BlendState = blendDesc;
			pipelineDescDT.DepthStencilState = depthStencilDesc;
			pipelineDescDT.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			pipelineDescDT.SampleMask = UINT_MAX;
			pipelineDescDT.SampleDesc.Count = 1;
			pipelineDescDT.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; //GSでポイントリストを三角形リストに変換するため、プリミティブトポロジーはポイント
			pipelineDescDT.NumRenderTargets = 1;
			pipelineDescDT.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			HRESULT hr = Device->CreateGraphicsPipelineState(&pipelineDescDT, IID_PPV_ARGS(PipelineStateDT.GetAddressOf()));
			assert(SUCCEEDED(hr));
		}
	} {}

	//出力領域を設定
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = (float)ClientWidth;
	Viewport.Height = (float)ClientHeight;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	//切り取り矩形を設定
	ScissorRect.left = 0;
	ScissorRect.top = 0;
	ScissorRect.right = ClientWidth;
	ScissorRect.bottom = ClientHeight;


	return S_OK;
}

HRESULT Graphic::createD2D()
{
	ComPtr<ID3D11Device> d3d11Device;

	//D3D11 生成フラグ
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG
	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//対応する機能レベル
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	//D3D11On12デバイスの生成
	HRESULT hr = D3D11On12CreateDevice(
		Device.Get(),
		d3d11DeviceFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		reinterpret_cast<IUnknown**>(mCommandQueue.GetAddressOf()),
		1,
		0,
		d3d11Device.ReleaseAndGetAddressOf(),
		mD3D11DeviceContext.ReleaseAndGetAddressOf(),
		nullptr
	);
	assert(SUCCEEDED(hr));
	d3d11Device.As(&mD3D11On12Device);

	//DXGIデバイスを取得
	ComPtr<IDXGIDevice> dxgiDevice;
	mD3D11On12Device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));

	//D2D1ファクトリの生成
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		mD2DFactory.ReleaseAndGetAddressOf()
	);
	assert(SUCCEEDED(hr));

	//DWriteファクトリの生成
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		&mDWriteFactory
	);

	//D2D1デバイスの生成
	hr = mD2DFactory->CreateDevice(
		dxgiDevice.Get(),
		mD2DDevice.ReleaseAndGetAddressOf()
	);
	assert(SUCCEEDED(hr));

	//D2D1デバイスコンテキストの生成
	hr = mD2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		mD2DDeviceContext.ReleaseAndGetAddressOf()
	);

	float dpi = GetDpiForWindow(hWnd);

	//ビットマッププロパティ
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			dpi,
			dpi
		);
	for (int i = 0; i < 2; i++) {
		//バックバッファの取得
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		hr = mD3D11On12Device->CreateWrappedResource(
			BackBuffers[i].Get(),
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&mWrappedBackBuffers[i])
		);
		ComPtr<IDXGISurface> dxgiBackBuffer;
		hr = mWrappedBackBuffers[i]->QueryInterface(IID_PPV_ARGS(&dxgiBackBuffer));
		assert(SUCCEEDED(hr));

		//D2D1のビットマップを生成
		hr = mD2DDeviceContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			mD2DRenderTargets[i].ReleaseAndGetAddressOf()
		);
		assert(SUCCEEDED(hr));

	}
	
	return S_OK;
}

HRESULT Graphic::createCbvAndHeap()
{
	HRESULT hr = createCbvTbvHeap(mCbvTbvHeap, 10000); //共有用のヒープを作成
	for(int frame = 0; frame < FrameCount; frame++) {
		hr = createBuf(1 << 20, mConstantBuf[frame]);	//コンスタントバッファを1MB作成
		assert(SUCCEEDED(hr));
	}
	for (int frame = 0; frame < FrameCount; frame++) {
		hr = mConstantBuf[frame]->Map(0, nullptr, reinterpret_cast<void**>(&mConstantData[frame]));	//マップする
		assert(SUCCEEDED(hr));
	}

	return hr;
}

void Graphic::updateBase3DData()
{
	//光源の更新
	updateSpotLight(mGame->getSpotLights());
	updatePointLight(mGame->getPointLights());
	//更新したデータをコンスタントバッファへコピー
	for(int i = 0; i < FrameCount; i++)
	memcpy(mConstantData[i], &Base3DData, sizeof(Base3DData));

}

HRESULT Graphic::createBuf(UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer)
{
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD; //CPUからGPUへ書き込み
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //一次元配列
	desc.Alignment = 0;
	desc.Width = sizeInBytes; //バッファのサイズ
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	HRESULT hr = Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(buffer.ReleaseAndGetAddressOf())
	);

	return hr;
}

HRESULT Graphic::updateBuf(void* data, UINT sizeInBytes, ComPtr<ID3D12Resource>& buffer)
{
	UINT8* mappedBuf; //コピー先のアドレス
	HRESULT hr = buffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
	if (FAILED(hr))
	{
		return hr;
	}
	memcpy(mappedBuf, data, sizeInBytes);
	buffer->Unmap(0, nullptr);
	return hr;
}

HRESULT Graphic::mapBuf(void** mappedBuf, ComPtr<ID3D12Resource>& buffer)
{
	HRESULT hr = buffer->Map(0, nullptr, mappedBuf);
	return hr;
}

void Graphic::unmapBuf(ComPtr<ID3D12Resource>& buffer)
{
	buffer->Unmap(0, nullptr);
}

UINT Graphic::alignedSize(UINT size)
{
	return (size + 0xff) & ~0xff;
}

HRESULT Graphic::createShaderResource(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource)
{



	//ファイルを読み込み、生データを取り出す
	unsigned char* pixels = nullptr;
	int width = 0, height = 0, bytePerPixel = 4;
	pixels = stbi_load(filename.c_str(), &width, &height, nullptr, bytePerPixel);
	assert(pixels != nullptr);

	//１行のピッチを256の倍数にしておく(バッファサイズは256の倍数でなければいけない)
	const UINT64 alignedRowPitch = (width * bytePerPixel + 0xff) & ~0xff;

	//アップロード用中間バッファをつくり、生データをコピーしておく
	ComPtr<ID3D12Resource> uploadBuf;
	{
		//テクスチャではなくフツーのバッファとしてつくる
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = alignedRowPitch * height;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = {1,0};//通常テクスチャなのでアンチェリしない
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		HRESULT hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuf));
		assert(SUCCEEDED(hr));

		//生データをuploadbuffに一旦コピーします
		uint8_t* mapBuf = nullptr;
		hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//マップ
		auto srcAddress = pixels;
		auto originalRowPitch = width * bytePerPixel;
		for (int y = 0; y < height; ++y) {
			memcpy(mapBuf, srcAddress, originalRowPitch);
			//1行ごとの辻褄を合わせてやる
			srcAddress += originalRowPitch;
			mapBuf += alignedRowPitch;
		}
		uploadBuf->Unmap(0, nullptr);//アンマップ
	}

	//そして、最終コピー先であるテクスチャバッファ_bを作る
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		HRESULT hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(shaderResource.ReleaseAndGetAddressOf()));
		assert(SUCCEEDED(hr));
	}
	//uploadBufからtextureBufへコピーする長い道のりが始まります

	//まずコピー元ロケーションの準備・フットプリント指定
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuf.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(1);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignedRowPitch);
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//コピー先ロケーションの準備・サブリソースインデックス指定
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = shaderResource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	//コマンドリストでコピーを予約しますよ！！！
	mLoadList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	//コピー先からテクスチャリソースに切り替える
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = shaderResource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	mLoadList->ResourceBarrier(1, &barrier);
	//コマンドリストを閉じて
	mLoadList->Close();
	//実行
	ID3D12CommandList* commandLists[] = { mLoadList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	//リソースがGPUに転送されるまで待機する
	UINT64 fenceValue = mFenceValue++;
	mCommandQueue->Signal(mFence.Get(), fenceValue);
	if (mFence->GetCompletedValue() < fenceValue) {
		mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	//テクスチャロード用の一次アロケータ
	mLoadAllocator->Reset();
	mLoadList->Reset(mLoadAllocator.Get(), nullptr);

	//開放
	stbi_image_free(pixels);

	return true;
}

XMFLOAT2 Graphic::createShaderResourceGetSize(const std::string& filename, ComPtr<ID3D12Resource>& shaderResource)
{


	//ファイルを読み込み、生データを取り出す
	unsigned char* pixels = nullptr;
	int width = 0, height = 0, bytePerPixel = 4;
	pixels = stbi_load(filename.c_str(), &width, &height, nullptr, bytePerPixel);
	assert(pixels != nullptr);

	//１行のピッチを256の倍数にしておく(バッファサイズは256の倍数でなければいけない)
	const UINT64 alignedRowPitch = (width * bytePerPixel + 0xff) & ~0xff;

	//アップロード用中間バッファをつくり、生データをコピーしておく
	ComPtr<ID3D12Resource> uploadBuf;
	{
		//テクスチャではなくフツーのバッファとしてつくる
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = alignedRowPitch * height;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc = { 1,0 };//通常テクスチャなのでアンチェリしない
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		HRESULT hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuf));
		assert(SUCCEEDED(hr));

		//生データをuploadbuffに一旦コピーします
		uint8_t* mapBuf = nullptr;
		hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//マップ
		auto srcAddress = pixels;
		auto originalRowPitch = width * bytePerPixel;
		for (int y = 0; y < height; ++y) {
			memcpy(mapBuf, srcAddress, originalRowPitch);
			//1行ごとの辻褄を合わせてやる
			srcAddress += originalRowPitch;
			mapBuf += alignedRowPitch;
		}
		uploadBuf->Unmap(0, nullptr);//アンマップ
	}

	//そして、最終コピー先であるテクスチャバッファ_bを作る
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		HRESULT hr = Device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(shaderResource.ReleaseAndGetAddressOf()));
		assert(SUCCEEDED(hr));
	}
	//uploadBufからtextureBufへコピーする長い道のりが始まります

	//まずコピー元ロケーションの準備・フットプリント指定
	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadBuf.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(1);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignedRowPitch);
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//コピー先ロケーションの準備・サブリソースインデックス指定
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = shaderResource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	//コマンドリストでコピーを予約しますよ！！！
	mLoadList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	//コピー先からテクスチャリソースに切り替える
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = shaderResource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	mLoadList->ResourceBarrier(1, &barrier);
	//コマンドリストを閉じて
	mLoadList->Close();
	//実行
	ID3D12CommandList* commandLists[] = { mLoadList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	UINT64 fenceValue = mFenceValue++;
	mCommandQueue->Signal(mFence.Get(), fenceValue);
	if (mFence->GetCompletedValue() < fenceValue) {
		mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	//テクスチャロード用の一次アロケータ
	mLoadAllocator->Reset();
	mLoadList->Reset(mLoadAllocator.Get(), nullptr);

	//開放
	stbi_image_free(pixels);

	return XMFLOAT2((float)width, (float)height);
}

HRESULT Graphic::createCbvTbvHeap(ComPtr<ID3D12DescriptorHeap>& cbvTbvHeap, UINT numDescriptors)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = numDescriptors;
	desc.NodeMask = 0;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	return Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(cbvTbvHeap.ReleaseAndGetAddressOf()));
	
}

void Graphic::createVertexBufferView(ComPtr<ID3D12Resource>& vertexBuf, UINT sizeInBytes, UINT strideInBytes, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
{
	vertexBufferView.BufferLocation = vertexBuf->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeInBytes; //全バイト数
	vertexBufferView.StrideInBytes = strideInBytes; //1頂点のバイト数

}

void Graphic::createIndexBufferView(ComPtr<ID3D12Resource>& indexBuf, UINT sizeInBytes, D3D12_INDEX_BUFFER_VIEW& indexBufferView)
{
	indexBufferView.BufferLocation = indexBuf->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeInBytes; //全バイト数
	indexBufferView.Format = DXGI_FORMAT_R16_UINT; //1頂点のバイト数
}

void Graphic::createConstantBufferView(ComPtr<ID3D12Resource>& constantBuf, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = constantBuf->GetGPUVirtualAddress();
	desc.SizeInBytes = static_cast<UINT>(constantBuf->GetDesc().Width); //256バイトアライメント
	Device->CreateConstantBufferView(&desc, handle);
}

void Graphic::createConstantBufferView(int cbIndex, int cbSize, int heapIndex, int heapSize)
{
	//二つのコンスタントバッファ分ビューを作成する
	for (int frame = 0; frame < FrameCount; frame++) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = mConstantBuf[frame]->GetGPUVirtualAddress() + cbIndex;
		desc.SizeInBytes = static_cast<UINT>(cbSize); //256バイトアライメント

		auto hCbvTbvHeap = mCbvTbvHeap->GetCPUDescriptorHandleForHeapStart();
		hCbvTbvHeap.ptr += getCbvTbvIncSize() * (heapIndex + frame * heapSize);

		Device->CreateConstantBufferView(&desc, hCbvTbvHeap);
	}
}

void Graphic::createBase3DBufferView(int heapIndex, int heapSize)
{
	createConstantBufferView(0, alignedSize(sizeof(Base3DData)), heapIndex, heapSize);
}

void Graphic::createShaderResourceView(ComPtr<ID3D12Resource>& shaderResource, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = shaderResource->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	desc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	Device->CreateShaderResourceView(shaderResource.Get(), &desc, handle);
}

void Graphic::createShaderResourceView(ID3D12Resource* shaderResource, int heapIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = shaderResource->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	desc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	auto hCbvTbvHeap = mCbvTbvHeap->GetCPUDescriptorHandleForHeapStart();
	hCbvTbvHeap.ptr += getCbvTbvIncSize() * heapIndex;

	Device->CreateShaderResourceView(shaderResource, &desc, hCbvTbvHeap);
}

void Graphic::updateViewProj(XMMATRIX& viewProj)
{
	Base3DData.viewProj = viewProj;
}

void Graphic::updatePointLight(const std::vector<PointLightComponent*>& lights)
{
	for (int i = 0; i < lights.size(); i++) {
		Base3DData.pointLights[i].position = lights[i]->getPosition();
		Base3DData.pointLights[i].color = lights[i]->getColor();
		Base3DData.pointLights[i].setValue.x = lights[i]->getActive();
		Base3DData.pointLights[i].setValue.y = lights[i]->getIntensity();
		Base3DData.pointLights[i].setValue.z = lights[i]->getRange();
	}
}

void Graphic::updateSpotLight(const std::vector<SpotLightComponent*>& lights)
{
	for (int i = 0; i < lights.size(); i++) {
		Base3DData.spotLights[i].position = lights[i]->getPosition();
		Base3DData.spotLights[i].direction = lights[i]->getDirection();
		Base3DData.spotLights[i].color = lights[i]->getColor();
		Base3DData.spotLights[i].setValue.x = lights[i]->getActive();
		Base3DData.spotLights[i].setValue.y = lights[i]->getIntensity();
		Base3DData.spotLights[i].setValue.z = lights[i]->getRange();
		Base3DData.spotLights[i].attAngle.x = lights[i]->getUAngle();
		Base3DData.spotLights[i].attAngle.y = lights[i]->getPAngle();
	}
}

void Graphic::updateCameraPos(XMFLOAT4& cameraPos)
{
	Base3DData.cameraPos = cameraPos;
}

void Graphic::updateDamageFlashIntensity(float intensity)
{
	Base3DData.playerFlashIntensity = intensity;
}

void Graphic::clearColor(float r, float g, float b)
{
	ClearColor[0] = r, ClearColor[1] = g, ClearColor[2] = b;
}

void Graphic::begin3DRender()
{
	//現在のバックバッファのインデックスを取得。このプログラムの場合0 or 1になる。
	//BackBufIdx = SwapChain->GetCurrentBackBufferIndex();

	//バリアでバックバッファを描画ターゲットに切り替える
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//このバリアは状態遷移タイプ
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = BackBuffers[BackBufIdx].Get();//リソースはバックバッファ
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//遷移前はPresent
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//遷移後は描画ターゲット
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);

	//バックバッファの場所を指すディスクリプタヒープハンドルを用意する
	auto hBbvHeap = BbvHeap->GetCPUDescriptorHandleForHeapStart();
	hBbvHeap.ptr += BackBufIdx * BbvHeapSize;
	//デプスステンシルバッファのディスクリプタハンドルを用意する
	auto hDsvHeap = DsvHeap->GetCPUDescriptorHandleForHeapStart();
	//バックバッファとデプスステンシルバッファを描画ターゲットとして設定する
	mCommandList->OMSetRenderTargets(1, &hBbvHeap, false, &hDsvHeap);

	//描画ターゲットをクリアする
	mCommandList->ClearRenderTargetView(hBbvHeap, ClearColor, 0, nullptr);

	//デプスステンシルバッファをクリアする
	mCommandList->ClearDepthStencilView(hDsvHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	//ビューポートとシザー矩形をセット
	mCommandList->RSSetViewports(1, &Viewport);
	mCommandList->RSSetScissorRects(1, &ScissorRect);

	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//三角形リスト

	//ディスクリプタヒープをＧＰＵにセット
	UINT numDescriptorHeaps = 1;
	mCommandList->SetDescriptorHeaps(numDescriptorHeaps, mCbvTbvHeap.GetAddressOf());

}

void Graphic::end3DRender()
{
	
	//バリアでバックバッファを表示用に切り替える
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//このバリアは状態遷移タイプ
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = BackBuffers[BackBufIdx].Get();//リソースはバックバッファ
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//遷移前はPresent
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//遷移後は描画ターゲット
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);

	//コマンドリストをクローズする
	mCommandList->Close();
	//コマンドリストを実行する
	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

}

void Graphic::moveToNextFrame()
{
	//バックバッファを表示
	SwapChain->Present(0, 0);

	//現フレームのフェンス値を記録
	mFenceValues[BackBufIdx] = mFenceValue;
	mCommandQueue->Signal(mFence.Get(), mFenceValue); //GPUの描画が終わったらmFenceValueを出力
	mFenceValue++;

	//次フレームのバックバッファインデックスを取得
	UINT nextBufIdx = SwapChain->GetCurrentBackBufferIndex();

	//次フレームのバッファをGPUがまだ使っていれば待機
	if (mFence->GetCompletedValue() < mFenceValues[nextBufIdx]) {
		//前のフレームのフェンス値になるまで待つ
		mFence->SetEventOnCompletion(mFenceValues[nextBufIdx], mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}


	//コマンドアロケータをリセット
	Hr = mCommandAllocator[nextBufIdx]->Reset();
	assert(SUCCEEDED(Hr));
	//コマンドリストをリセット
	Hr = mCommandList->Reset(mCommandAllocator[nextBufIdx].Get(), nullptr);
	assert(SUCCEEDED(Hr));

	BackBufIdx = nextBufIdx;
}

void Graphic::prepareCommandList()
{
	mCommandList->Close();
	mCommandAllocator[BackBufIdx]->Reset();
	mCommandList->Reset(mCommandAllocator[BackBufIdx].Get(), nullptr);
}

bool Graphic::quit()
{
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
		if (Msg.message == WM_QUIT) return true;
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return false;
}

int Graphic::msg_wparam()
{
	return (int)Msg.wParam;
}

void Graphic::closeEventHandle()
{
	CloseHandle(mFenceEvent);
}

void Graphic::waitGPU()
{
	//現在のFence値がコマンド中菱後にFenceに書き込まれるようにス
	UINT64 fvalue = mFenceValue;
	mCommandQueue->Signal(mFence.Get(), fvalue);
	mFenceValue++;

	//まだコマンドキューが終了していないことを確認する
	if (mFence->GetCompletedValue() < fvalue) {
		//このFenceにおいて、fvalueの値になったらイベントを発生させる
		mFence->SetEventOnCompletion(fvalue, mFenceEvent);
		//イベントが発生するまで待つ
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

float Graphic::getAspect()
{
	return Aspect;
}

UINT Graphic::getCbvTbvIncSize()
{
	return Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

ID3D12GraphicsCommandList* Graphic::getCommandList()
{
	return mCommandList.Get();
}

ID3D12CommandQueue* Graphic::getCommandQueue()
{
	return mCommandQueue.Get();
}

ID3D12CommandAllocator* Graphic::getCommandAllocator()
{
	return mCommandAllocator[BackBufIdx].Get();
}

ID3D12Device* Graphic::getDevice()
{
	return Device.Get();
}

float Graphic::getClientWidth()
{
	return ClientWidth;
}

float Graphic::getClientHeight()
{
	return ClientHeight;
}

ID3D11On12Device* Graphic::getD3D11On12Device()
{
	return mD3D11On12Device.Get();
}

ID3D11DeviceContext* Graphic::getD3D11DeviceContext()
{
	return mD3D11DeviceContext.Get();
}

ID2D1DeviceContext* Graphic::getD2DDeviceContext()
{
	return mD2DDeviceContext.Get();
}

IDWriteFactory* Graphic::getDWriteFactory()
{
	return mDWriteFactory.Get();
}

ID2D1Bitmap1* Graphic::getD2DRenderTarget()
{
	return mD2DRenderTargets[BackBufIdx].Get();
}

UINT8* Graphic::getConstantData()
{
	return mConstantData[BackBufIdx];
}

UINT8* Graphic::getConstantData(int frame)
{
	return mConstantData[frame];
}

D3D12_GPU_DESCRIPTOR_HANDLE Graphic::getHeapHandle()
{
	return mCbvTbvHeap->GetGPUDescriptorHandleForHeapStart();
}

int Graphic::getBackBufIdx()
{
	return BackBufIdx;
}

void Graphic::setRenderType(STATE state)
{
	//mStateに応じて3Dと2Dを切換え
	if (state == Graphic::RENDER_3D) {
		//パイプラインステートをセット
		mCommandList->SetPipelineState(PipelineState.Get());
		//ルートシグニチャをセット
		mCommandList->SetGraphicsRootSignature(RootSignature.Get());
	}
	else if (state == Graphic::RENDER_2D) {
		//パイプラインステートをセット
		mCommandList->SetPipelineState(PipelineState2D.Get());
		//ルートシグニチャをセット
		mCommandList->SetGraphicsRootSignature(RootSignature2D.Get());
	}
	else if (state == Graphic::RENDER_DT) {
		//パイプラインステートをセット
		mCommandList->SetPipelineState(PipelineStateDT.Get());
		//ルートシグニチャをセット
		mCommandList->SetGraphicsRootSignature(RootSignatureDT.Get());
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

