#pragma once

#include "Graphic/Core/Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include <d3dx12.h>

class PipelineStateBuilder
{
public:
	PipelineStateBuilder(); //CD3DX12_DEFAULTで妥当な既定値を入れておく
	~PipelineStateBuilder() = default;

	PipelineStateBuilder& setRootSignature(ID3D12RootSignature* rootSignature);
	PipelineStateBuilder& setVertexShader(const std::string& path);
	PipelineStateBuilder& setPixelShader(const std::string& path);
	PipelineStateBuilder& setGeometoryShader(const std::string& path);
	PipelineStateBuilder& setInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements);
	PipelineStateBuilder& setBlendState(const D3D12_BLEND_DESC& blend);
	PipelineStateBuilder& setRasterizerState(const D3D12_RASTERIZER_DESC& rasterizer);
	PipelineStateBuilder& setDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencil);
	PipelineStateBuilder& setRenderTargetFormat(DXGI_FORMAT format, UINT index = 0);
	PipelineStateBuilder& setNumRenderTargets(UINT num);
	PipelineStateBuilder& setDepthStencilFormat(DXGI_FORMAT format);
	PipelineStateBuilder& setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);
	PipelineStateBuilder& setSampleDesc(UINT count, UINT quality = 0);

	ComPtr<ID3D12PipelineState> build(ID3D12Device& device);

private:
	//シェーダーバイトコードの実体を保持(build()実行までポインタを生かしておく必要がある)
	std::unique_ptr<class BIN_FILE12> mVertexShaderFile;
	std::unique_ptr<class BIN_FILE12> mPixelShaderFile;
	std::unique_ptr<class BIN_FILE12> mGeometoryShaderFile;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputElements; //同様にアドレス保持のため保存

	D3D12_GRAPHICS_PIPELINE_STATE_DESC mDesc = {};
};

