#pragma once

#include "Definition.h"
#include <wrl/client.h>
#include <winuser.h>
#include "directx/d3dx12.h"
#include <deque>

class RootSignatureBuilder
{
public:
	RootSignatureBuilder() = default;

	RootSignatureBuilder& addDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>& range, D3D12_SHADER_VISIBILITY visibility);
	
	//CBV/SRV/UAVを1レンジ1テーブルとしてルートパラメータに追加
	RootSignatureBuilder& addCBVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
	RootSignatureBuilder& addSRVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);
	RootSignatureBuilder& addUAVTable(UINT baseShaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);

	//静的サンプラー
	RootSignatureBuilder& addStaticSampler(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility);

	//頻繁に変わる小さい値をディスクリプタ経由せず直接埋め込む場合
	RootSignatureBuilder& addRootConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility, UINT registerSpace = 0);

	RootSignatureBuilder& setFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);

	ComPtr<ID3D12RootSignature> build(ID3D12Device& device);

private:
	std::deque<std::vector<D3D12_DESCRIPTOR_RANGE>> mRangeStorage;	//D3D12_DESCRIPTOR_RANGEのアドレス固定用

	std::vector<D3D12_ROOT_PARAMETER> mRootParams;
	std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;
	D3D12_ROOT_SIGNATURE_FLAGS mFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
};

