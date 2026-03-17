#include "SpriteComponent.h"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "Graphic.h"
#include "Actor.h"
#include "Game.h"
#include "AssetManager.h"

SpriteComponent::SpriteComponent(Actor* owner, float zDepth) : Component(owner)
{
	mPosition = { 0.0f, 0.0f, zDepth};
	mScale = { 1.0f, 1.0f };
	mSpriteSize = { 100.0f, 100.0f };
	mBordarSize = 0.0f;
	mRotation = 0.0f;

	mGraphic = mOwner->getGame()->getGraphic();
	mAssetManager = mOwner->getGame()->getAssetManager();
	mCommandList = mGraphic->getCommandList();
	mOwner->getGame()->addSprite(this);
	mNumSprites = 1;
}

SpriteComponent::~SpriteComponent()
{
}

void SpriteComponent::endProccess()
{
	//Gameからスプライトを削除
	mOwner->getGame()->removeSprite(this);
}

void SpriteComponent::create(const std::string filename)
{
	//コンスタントバッファ、ディスクリプタヒープ用のインデックスを取得
	mCBSize = 256 * (1 + mNumSprites); //spriteConstantBuf + textureの数
	mHeapSize = 2 + mNumSprites;
	mCBIndex = mAssetManager->getCBEndIndex(mCBSize);
	mHeapIndex = mAssetManager->getHeapEndIndex(mHeapSize);

	//Sprite用の各Viewを取得
	SpriteData spriteData = mAssetManager->getSpriteData();
	mVertexBufView = spriteData.VertexBufView;
	mIndexBufView = spriteData.IndexBufView;

	//テクスチャを取得
	mTextureSize = mAssetManager->createTextureAndGetSize(filename);
	mTextureBuf = mAssetManager->getShaderResource(filename);

	//コンスタントバッファの初期化
	Cb3.windowSize = XMFLOAT2(
		(float)mGraphic->getClientWidth(),
		(float)mGraphic->getClientHeight()
	);
	Cb3.spriteSize = mSpriteSize;
	Cb3.textureSize = mTextureSize;
	Cb3.bordarSize = mBordarSize;
	memcpy(mGraphic->getConstantData() + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//ディスクリプタヒープにViewを作る
	int heapIndex = mHeapIndex;
	mGraphic->createConstantBufferView(mCBIndex, 256, heapIndex, 1); heapIndex += 2;
	mGraphic->createShaderResourceView(mTextureBuf, heapIndex);
}

void SpriteComponent::draw()
{
	//コンスタントバッファの更新
	//ワールドマトリックス
	XMMATRIX world = XMMatrixIdentity()
		* XMMatrixScaling(mScale.x, mScale.y, 1.0f)
		* XMMatrixTranslation(-mSpriteSize.x * 0.5f, -mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixRotationZ(mRotation)
		* XMMatrixTranslation(mSpriteSize.x * 0.5f, mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z)
		;
	Cb3.world = world;
	Cb3.spriteSize = mSpriteSize;	//スプライトサイズ
	Cb3.bordarSize = mBordarSize;	//ボーダーサイズ
	//コンスタントバッファへコピー
	memcpy(mGraphic->getConstantData() + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//頂点をセット
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hCbvTbvHeap = mGraphic->getHeapHandle();
	UINT CbvTbvSize = mGraphic->getCbvTbvIncSize();
	hCbvTbvHeap.ptr += (mHeapIndex + mGraphic->getBackBufIdx()) * CbvTbvSize;

	mCommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	hCbvTbvHeap = mGraphic->getHeapHandle();
	hCbvTbvHeap.ptr += (mHeapIndex + 2) * CbvTbvSize;
	mCommandList->SetGraphicsRootDescriptorTable(1, hCbvTbvHeap);
	//描画。インデックスを使用
	mCommandList->IASetIndexBuffer(&mIndexBufView);
	mCommandList->DrawIndexedInstanced(mAssetManager->getSpriteIndicesSize(), 1, 0, 0, 0);
	
}

void SpriteComponent::setPosition(const XMFLOAT3& position)
{
	mPosition = position;
}

void SpriteComponent::setScale(const XMFLOAT2& scale)
{
	mScale = scale;
}

void SpriteComponent::setRotation(const float rotation)
{
	mRotation = rotation;
}

void SpriteComponent::setSpriteSize(const XMFLOAT2& size)
{
	mSpriteSize = size;
}

void SpriteComponent::setBordarSize(const float size)
{
	mBordarSize = size;
}

void SpriteComponent::movePositon(const XMFLOAT2& diff)
{
	mPosition.x += diff.x;
	mPosition.y += diff.y;
}
