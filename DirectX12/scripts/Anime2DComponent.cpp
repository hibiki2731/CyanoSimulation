#include "Anime2DComponent.h"
#include "Game.h"
#include "Actor.h"
#include "AssetManager.h"
#include <string>

Anime2DComponent::Anime2DComponent(Actor* owner, float zDepth)
	: SpriteComponent(owner, zDepth)
{
	mTextureIndex = 0;
}

void Anime2DComponent::create(const std::string filename, int textureNum)
{

	//コンスタントバッファ、ディスクリプタヒープ用のインデックスを取得
	mCBSize = 256 * (1 + textureNum); //spriteConstantBuf + textureの数
	mHeapSize = 2 + textureNum;
	mCBIndex = mAssetManager->getCBEndIndex(mCBSize);
	mHeapIndex = mAssetManager->getHeapEndIndex(mHeapSize);

	//Sprite用の各Viewを取得
	SpriteData spriteData= mAssetManager->getSpriteData();
	mVertexBufView = spriteData.VertexBufView;
	mIndexBufView = spriteData.IndexBufView;

	//テクスチャを取得
	mTextureNum = textureNum;
	mTextureBufs.resize(mTextureNum);
	int dotPos = filename.rfind('.');
	std::string preName = filename.substr(0, dotPos);
	std::string postName = filename.substr(dotPos);
	for (int i = 0;i < mTextureNum;i++)
	{
		std::string textureName = preName + std::to_string(i) + postName;
		mTextureSize = mAssetManager->createTextureAndGetSize(textureName);
		mTextureBufs[i] = mAssetManager->getShaderResource(textureName);
	}
	
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
	auto heapIndex = mHeapIndex;
	mGraphic->createConstantBufferView(mCBIndex, 256, heapIndex, 1); heapIndex += 2;
	for (int i = 0; i < mTextureNum; i++) mGraphic->createShaderResourceView(mTextureBufs[i], heapIndex); heapIndex++;

}

void Anime2DComponent::draw()
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
\
	//頂点をセット
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hCbvTbvHeap = mGraphic->getHeapHandle();
	UINT CbvTbvSize = mGraphic->getCbvTbvIncSize();
	hCbvTbvHeap.ptr += (mHeapIndex + mGraphic->getBackBufIdx()) * CbvTbvSize;

	mCommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	hCbvTbvHeap = mGraphic->getHeapHandle();
	hCbvTbvHeap.ptr += CbvTbvSize * (mHeapIndex + 2 + mTextureIndex);
	mCommandList->SetGraphicsRootDescriptorTable(1, hCbvTbvHeap);
	//描画。インデックスを使用
	mCommandList->IASetIndexBuffer(&mIndexBufView);
	mCommandList->DrawIndexedInstanced(mAssetManager->getSpriteIndicesSize(), 1, 0, 0, 0);
}

void Anime2DComponent::endProccess()
{
	mOwner->getGame()->removeSprite(this);
}

void Anime2DComponent::setTextureIndex(int index)
{
	if (index < 0) index = 0;
	if (index >= mTextureNum) index = mTextureNum - 1;
	mTextureIndex = index;
}
