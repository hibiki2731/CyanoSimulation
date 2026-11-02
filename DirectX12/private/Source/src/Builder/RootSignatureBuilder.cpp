#include "Builder/RootSignatureBuilder.h"

RootSignatureBuilder& RootSignatureBuilder::addDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>& range, D3D12_SHADER_VISIBILITY visibility)
{
	//D3D12_DESCRIPTOR_RANGEの実体をdequeに保存
	mRangeStorage.push_back(range);
	const auto& storedRange = mRangeStorage.back();

	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable.pDescriptorRanges = storedRange.data();
	param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(storedRange.size());
	param.ShaderVisibility = visibility;

	mRootParams.push_back(param);
	return *this;

}

RootSignatureBuilder& RootSignatureBuilder::addCBVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range.BaseShaderRegister = baseShaderRegister;
	range.NumDescriptors = numDescriptors;
	range.RegisterSpace = registerSpace;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	return addDescriptorTable({ range }, visibility);
}

RootSignatureBuilder& RootSignatureBuilder::addSRVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.BaseShaderRegister = baseShaderRegister;
	range.NumDescriptors = numDescriptors;
	range.RegisterSpace = registerSpace;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	return addDescriptorTable({ range }, visibility);
}

RootSignatureBuilder& RootSignatureBuilder::addUAVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range.BaseShaderRegister = baseShaderRegister;
	range.NumDescriptors = numDescriptors;
	range.RegisterSpace = registerSpace;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	return addDescriptorTable({ range }, visibility);
}

RootSignatureBuilder& RootSignatureBuilder::addStaticSampler(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = filter;
	sampler.AddressU = addressMode;
	sampler.AddressV = addressMode;
	sampler.AddressW = addressMode;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.MinLOD = 0.0f;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.ShaderRegister = shaderRegister;
	sampler.ShaderVisibility = visibility;
	mStaticSamplers.push_back(sampler);
	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::addRootConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace)
{
	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	param.Constants.ShaderRegister = shaderRegister;
	param.Constants.RegisterSpace = registerSpace;
	param.Constants.Num32BitValues = num32BitValues;
	param.ShaderVisibility = visibility;
	mRootParams.push_back(param);
	return *this;
}

RootSignatureBuilder& RootSignatureBuilder::setFlags(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	mFlags = flags;
	return *this;
}

ComPtr<ID3D12RootSignature> RootSignatureBuilder::build(ID3D12Device& device)
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = static_cast<UINT>(mRootParams.size());
	desc.pParameters = mRootParams.data();
	desc.NumStaticSamplers = static_cast<UINT>(mStaticSamplers.size());
	desc.pStaticSamplers = mStaticSamplers.data();
	desc.Flags = mFlags;

	ComPtr<ID3DBlob> blob, errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.GetAddressOf(), errorBlob.GetAddressOf());
	if (FAILED(hr) && errorBlob) {
		OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
	}
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device.CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));
	return rootSignature;
}
