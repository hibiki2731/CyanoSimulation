#pragma once

#include "Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include "directx/d3dx12.h"

class Pipeline
{
public:
	Pipeline(ID3D12Device& device);

private:
	virtual std::vector<D3D12_DESCRIPTOR_RANGE> createRange() = 0;
	virtual D3D12_ROOT_PARAMETER createRootParamater(const std::vector<D3D12_DESCRIPTOR_RANGE>& range) = 0;
	virtual std::vector<D3D12_STATIC_SAMPLER_DESC> createSamplerDesc() = 0;
	virtual D3D12_ROOT_SIGNATURE_DESC createRootSigunatureDesc(const D3D12_ROOT_PARAMETER& rootParam, const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDesc) = 0;
	ComPtr<ID3DBlob> serializeRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc);
	void createRootSignature(ID3D12Device& device, const ComPtr<ID3DBlob>& blob);
	class BIN_FILE12 loadShader(const std::string& fileName);
	virtual D3D12_INPUT_ELEMENT_DESC createInputDesc() = 0;
	virtual D3D12_RASTERIZER_DESC createRasterDesc() = 0;
	virtual D3D12_BLEND_DESC createBlendDesc() = 0;
	virtual D3D12_DEPTH_STENCIL_DESC createDepthStencilDesc() = 0;
	virtual void createPipelineState(const D3D12_INPUT_ELEMENT_DESC& inputDesc, const D3D12_RASTERIZER_DESC& rasterDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc) = 0;


	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12PipelineState> mPipelineState;
};

