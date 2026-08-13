#pragma once
#include <memory>
#include "Graphic/Core/Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include <d3dx12.h>

class ComputePipelineStateBuilder
{
public:
	ComputePipelineStateBuilder(); //CD3DX12_DEFAULTで妥当な既定値を入れておく
	~ComputePipelineStateBuilder() = default;

	ComputePipelineStateBuilder& setRootSignature(ID3D12RootSignature* rootSignature);
	ComputePipelineStateBuilder& setComputeShader(const std::string& path);
	ComPtr<ID3D12PipelineState> build(ID3D12Device& device);

private:
	std::unique_ptr<class BIN_FILE12> mComputeShaderFile;

	D3D12_COMPUTE_PIPELINE_STATE_DESC mDesc = {};
};

