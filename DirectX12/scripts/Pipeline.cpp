#include "pch.h"
#include "Pipeline.h"
#include "BIN_FILE12.h"

Pipeline::Pipeline(ID3D12Device& device)
{

	std::vector<D3D12_DESCRIPTOR_RANGE> range = createRange();

	D3D12_ROOT_PARAMETER rootParam = createRootParamater(range);

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDesc = createSamplerDesc();

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = createRootSigunatureDesc(rootParam, samplerDesc);

	ComPtr<ID3DBlob> blob = serializeRootSignature(rootSignatureDesc);

	createRootSignature(device, blob);

	D3D12_INPUT_ELEMENT_DESC inputeDesc = createInputDesc();
	D3D12_RASTERIZER_DESC rasterDesc = createRasterDesc();
	D3D12_BLEND_DESC blendDesc = createBlendDesc();
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = createDepthStencilDesc();

	createPipelineState(
		inputeDesc,
		rasterDesc,
		blendDesc,
		depthStencilDesc);

}

ComPtr<ID3DBlob> Pipeline::serializeRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
{
		ComPtr<ID3DBlob> blob;
		HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), nullptr);
		assert(SUCCEEDED(hr));

		return blob;
}

void Pipeline::createRootSignature(ID3D12Device& device, const ComPtr<ID3DBlob>& blob)
{
		//ルートシグネチャの作成
		HRESULT hr = device.CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(mRootSignature.GetAddressOf()));
		assert(SUCCEEDED(hr));
}

BIN_FILE12 Pipeline::loadShader(const std::string& fileName)
{
	BIN_FILE12 file(fileName.c_str());
	assert(file.succeeded());

	return file;
}
