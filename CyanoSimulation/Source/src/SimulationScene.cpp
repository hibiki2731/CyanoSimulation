#include "SimulationScene.h"
#include "Editer/GUIDebugger.h"
#include "CyanoSimulator.h"
#include "GameLoopCore/Game.h"
#include "Builder/RootSignatureBuilder.h"
#include "Builder/PipelineStateBuilder.h"

SimulationScene::SimulationScene(Game& game)
	: Scene(game),
	mSimulator(nullptr)
{
}

void SimulationScene::onEnter()
{
	auto simulator = std::make_unique<CyanoSimulator>(*this);

#ifdef _DEBUG
	//mGame.getGUIDebugger().setCyanoSimulator(simulator.get());
#endif

	mSimulator = simulator.get();
	addActor(std::move(simulator));
	createCyanoGraphicsPSO(*mGame.getGraphic().getDevice());


#ifdef _DEBUG
	mDebugFlag = true;
#endif

}

void SimulationScene::onExit()
{
}

void SimulationScene::drawScene()
{
	auto commandList = mGame.getGraphic().getGraphicsCommandList();
	commandList->SetPipelineState(mCyanoGraphicsPSO.Get());
	commandList->SetGraphicsRootSignature(mCyanoGraphicsRootSignature.Get());
	mSimulator->draw();
}

void SimulationScene::createCyanoGraphicsPSO(ID3D12Device& device)
{
	UINT b0 = 0, u0 = 0, t0 = 0;
	mCyanoGraphicsRootSignature = RootSignatureBuilder()
		.addRootConstants(b0, 3, D3D12_SHADER_VISIBILITY_VERTEX)
		.addUAVTable(u0, 1, D3D12_SHADER_VISIBILITY_ALL)
		.addSRVTable(t0, 1, D3D12_SHADER_VISIBILITY_PIXEL)
		.addStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_SHADER_VISIBILITY_PIXEL)
		.setFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
		.build(device);

	//各種記述
	UINT slot0 = 0;
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescsCyano = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, slot0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterDesc.FrontCounterClockwise = true; //反時計回り

	D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;                  // 1.0
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;       // 1 - ソースのアルファ値
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;              // 加算
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	mCyanoGraphicsPSO = PipelineStateBuilder()
		.setRootSignature(mCyanoGraphicsRootSignature.Get())
		.setInputLayout(inputElementDescsCyano)
		.setVertexShader("Content/Shader/cso/CyanoVertexShader.cso")
		.setPixelShader("Content/Shader/cso/CyanoPixelShader.cso")
		.setRasterizerState(rasterDesc)
		.setBlendState(blendDesc)
		.setDepthStencilState(depthStencilDesc)
		.setDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
		.build(device);

}
