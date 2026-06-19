#include "pch.h"
#include "FadeGenerator.h"
#include "BIN_FILE12.h"
#include "Graphic.h"

FadeGenerator::FadeGenerator(Graphic& graphic)
	:mGraphic(graphic),
	currentFadeAlpha(0.0f),
	fadeTimer(0.0f),
	fadeOutDuration(0.0f),
	fadeInDuration(0.0f),
	isFadingIn(false),
	isFadingOut(false)
{
	//フェード用パイプラインステート
	{
		//ダメージエフェクト用ルートシグネチャ
		//ディスクリプタレンジ、ディスクリプタヒープとシェーダを紐づける役割を持つ
		D3D12_ROOT_PARAMETER rootParam[1] = {};
		rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParam[0].Constants.ShaderRegister = 0;
		rootParam[0].Constants.RegisterSpace = 0;
		rootParam[0].Constants.Num32BitValues = 1; //float
		rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //全てのシェーダから見える


		//ルートシグネチャ
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //入力アセンブラの入力レイアウトを許可
		desc.pParameters = rootParam;
		desc.NumParameters = _countof(rootParam);
		desc.pStaticSamplers = nullptr;  //サンプラーの先頭アドレス
		desc.NumStaticSamplers = 0; //サンプラーの数
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //入力アセンブラの入力レイアウトを許可

		//ルートシグネチャをシリアライズ(コンパイルするようなもの)
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), nullptr);
		assert(SUCCEEDED(hr));

		//ルートシグネチャの作成
		hr = mGraphic.getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(RootSignatureFade.GetAddressOf()));
		assert(SUCCEEDED(hr));

		{
			//シェーダの読み込み
			BIN_FILE12 vsFade("assets\\FadeVertexShader.cso");
			assert(vsFade.succeeded());
			BIN_FILE12 psFade("assets\\FadePixelShader.cso");
			assert(psFade.succeeded());

			//各種記述
			D3D12_INPUT_ELEMENT_DESC inputElementDescs2D = {};

			D3D12_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.FrontCounterClockwise = true; 
			rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterDesc.DepthClipEnable = TRUE;
			rasterDesc.MultisampleEnable = FALSE;
			rasterDesc.AntialiasedLineEnable = FALSE;
			rasterDesc.ForcedSampleCount = 0;
			rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
			blendDesc.BlendEnable = true;
			blendDesc.LogicOpEnable = FALSE;
			blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
			depthStencilDesc.DepthEnable = false;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; //書き込み許可
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; //小さいほうが手前
			depthStencilDesc.StencilEnable = FALSE; //ステンシルしない
			//ここまでの記述をまとめてパイプラインステートオブジェクトを作成
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
			pipelineDesc.pRootSignature = RootSignatureFade.Get();
			pipelineDesc.VS = { vsFade.code(), vsFade.size() };
			pipelineDesc.PS = { psFade.code(), psFade.size() };
			pipelineDesc.InputLayout = { nullptr, 0 };
			pipelineDesc.RasterizerState = rasterDesc;
			pipelineDesc.BlendState.RenderTarget[0] = blendDesc;
			pipelineDesc.DepthStencilState = depthStencilDesc;
			pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			pipelineDesc.SampleMask = UINT_MAX;
			pipelineDesc.SampleDesc.Count = 1;
			pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineDesc.NumRenderTargets = 1;
			pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			HRESULT hr = mGraphic.getDevice()->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(PipelineStateFade.GetAddressOf()));
			assert(SUCCEEDED(hr));
		}
	} {}

}

void FadeGenerator::startFadeIn(float duration)
{
	fadeTimer = 0.0f;
	isFadingIn = true;
	isFadingOut = false;
	fadeInDuration = duration;
}

void FadeGenerator::startFadeOut(float duration)
{
	fadeTimer = 0.0f;
	isFadingOut = true;
	fadeOutDuration = duration;
}

void FadeGenerator::renderFade()
{
	if (isFadingIn || isFadingOut) {
		mGraphic.getCommandList()->SetPipelineState(PipelineStateFade.Get());
		mGraphic.getCommandList()->SetGraphicsRootSignature(RootSignatureFade.Get());
		mGraphic.getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//ルートパラメータにフェードのアルファ値をセット
		mGraphic.getCommandList()->SetGraphicsRoot32BitConstants(0, 1, &currentFadeAlpha, 0);
		mGraphic.getCommandList()->DrawInstanced(3, 1, 0, 0);
	}
}

bool FadeGenerator::isFading()
{
	return isFadingIn || isFadingOut;
}

bool FadeGenerator::isFinishedFade()
{
	return (!isFadingIn && !isFadingOut) || (isFadingOut && currentFadeAlpha >= 1.0f);
}
