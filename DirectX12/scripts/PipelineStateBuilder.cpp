#include "pch.h"
#include "PipelineStateBuilder.h"
#include "BIN_FILE12.h"
#include "ComputePipelineStateBuilder.h"

PipelineStateBuilder::PipelineStateBuilder()
{	
	//デフォルト値を設定
	mDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	mDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	mDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	mDesc.SampleMask = UINT_MAX;
	mDesc.SampleDesc.Count = 1;
	mDesc.NumRenderTargets = 1;
	mDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	mDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

PipelineStateBuilder& PipelineStateBuilder::setRootSignature(ID3D12RootSignature* rootSignature)
{
	mDesc.pRootSignature = rootSignature;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setVertexShader(const std::string& path)
{
	mVertexShaderFile = std::make_unique<BIN_FILE12>(path.c_str());
	assert(mVertexShaderFile->succeeded());
	mDesc.VS = { mVertexShaderFile->code(), mVertexShaderFile->size() };
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setPixelShader(const std::string& path)
{
	mPixelShaderFile = std::make_unique<BIN_FILE12>(path.c_str());
	assert(mPixelShaderFile->succeeded());
	mDesc.PS = { mPixelShaderFile->code(), mPixelShaderFile->size() };
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setGeometoryShader(const std::string& path)
{
	mGeometoryShaderFile = std::make_unique<BIN_FILE12>(path.c_str());
	assert(mGeometoryShaderFile->succeeded());
	mDesc.GS = { mGeometoryShaderFile->code(), mGeometoryShaderFile->size() };
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements)
{
	mInputElements = elements; //実体を保持してポインタを安定させる
	mDesc.InputLayout = { mInputElements.data(), static_cast<UINT>(mInputElements.size()) };
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setBlendState(const D3D12_BLEND_DESC& blend)
{
	mDesc.BlendState = blend;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setRasterizerState(const D3D12_RASTERIZER_DESC& rasterizer)
{
	mDesc.RasterizerState = rasterizer;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencil)
{
	mDesc.DepthStencilState = depthStencil;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setRenderTargetFormat(DXGI_FORMAT format, UINT index)
{
	mDesc.RTVFormats[index] = format;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setNumRenderTargets(UINT num)
{
	mDesc.NumRenderTargets = num;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setDepthStencilFormat(DXGI_FORMAT format)
{
	mDesc.DSVFormat = format;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	mDesc.PrimitiveTopologyType = type;
	return *this;
}

PipelineStateBuilder& PipelineStateBuilder::setSampleDesc(UINT count, UINT quality)
{
	mDesc.SampleDesc.Count = count;
	mDesc.SampleDesc.Quality = quality;
	return *this;
}

ComPtr<ID3D12PipelineState> PipelineStateBuilder::build(ID3D12Device& device) 
{	
	assert(mDesc.pRootSignature != nullptr && "ルートシグネチャが設定されていません。");
	assert(mDesc.VS.pShaderBytecode != nullptr && "頂点シェーダーが設定されていません。");

	ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device.CreateGraphicsPipelineState(&mDesc, IID_PPV_ARGS(pso.GetAddressOf()));
	assert(SUCCEEDED(hr));
	return pso;
}
