#pragma once
#include "Definition.h"
#include "ConstantBuffer.h"
#include <DirectXMath.h>

struct TextCBSuballocationData
{
	XMMATRIX world;
	XMFLOAT2 windowSize; //xy:ウィンドウサイズ
	XMFLOAT2 spriteSize; //xy:サイズ (px)
	XMFLOAT2 textureSize; //xy:テクスチャサイズ (px)
	float bordarSize; //9スライス用ボーダーサイズ (px)
	float padding; //パディング
};

class TextCBSuballocation : public IConstantBufferSuballocation
{
public:
	TextCBSuballocation(const AlignedSizeInBytes& sizeInBytes, const ConstantBufferAddress& address, std::vector<void*>&& buffersOnCPU, std::vector<D3D12_GPU_VIRTUAL_ADDRESS>&& virtualAddressOnGPU);

	TextCBSuballocationData mData;

	void updateData() override;
	void updateWorld(const XMMATRIX& world, const int frame);

private:

};

