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

	void updateWorld(const XMMATRIX& world);
	void updateWindowSize(const XMFLOAT2& windowSize);
	void updateSpriteSize(const XMFLOAT2& spriteSize);
	void updateTextureSize(const XMFLOAT2& textureSize);
	void updateBordarSize(const float bordarSize);

	void applyChanges(const int frame) override;

private:
	TextCBSuballocationData mData;

};

