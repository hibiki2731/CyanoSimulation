#include "pch.h"
#include "ComputePipelineStateBuilder.h"
#include "BIN_FILE12.h"

ComputePipelineStateBuilder::ComputePipelineStateBuilder() {
}

ComputePipelineStateBuilder& ComputePipelineStateBuilder::setRootSignature(ID3D12RootSignature* rootSignature) {

	mDesc.pRootSignature = rootSignature;
	return *this;
}

ComputePipelineStateBuilder& ComputePipelineStateBuilder::setComputeShader(const std::string& path) {

	mComputeShaderFile = std::make_unique<BIN_FILE12>(path.c_str());
	assert(mComputeShaderFile->succeeded());
	mDesc.CS = { mComputeShaderFile->code(), mComputeShaderFile->size() };
	return *this;
}

ComPtr<ID3D12PipelineState> ComputePipelineStateBuilder::build(ID3D12Device& device) {

	assert(mDesc.pRootSignature != nullptr && "ルートシグネチャが設定されていません。");
	assert(mDesc.CS.pShaderBytecode != nullptr && "頂点シェーダーが設定されていません。");

	ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device.CreateComputePipelineState(&mDesc, IID_PPV_ARGS(pso.GetAddressOf()));
	assert(SUCCEEDED(hr));
	return pso;
}
