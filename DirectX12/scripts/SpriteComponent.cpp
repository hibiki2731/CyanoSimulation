#include "SpriteComponent.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "Graphic.h"
#include "Actor.h"
#include "Game.h"
#include "AssetManager.h"
#include "Scene.h"
#include <fstream>
#include "myJson.h"

SpriteComponent::SpriteComponent(Actor& owner, float zDepth) 
	: Component(owner),
	mGraphic(owner.getScene().getGame().getGraphic()),
	mAssetManager(owner.getScene().getGame().getAssetManager())
{
	mPosition = { 0.0f, 0.0f, zDepth};
	mScale = { 1.0f, 1.0f };
	mSpriteSize = { 100.0f, 100.0f };
	mBordarSize = 0.0f;
	mRotation = 0.0f;

	mCommandList = mGraphic.getCommandList();
	mOwner.getScene().addSprite(this);
	mNumSprites = 1;

	//初期化
	mCBIndex = 0;
	mHeapIndex = 0;
	mCBSize = 0;
	mHeapSize = 0;

	mTextureFilePath = "";
}

void SpriteComponent::loadFromJson(const nlohmann::json& json)
{
	//スプライトの作成
	auto filePath = json.value("filePath", "");
	if ( filePath != "")
		create(json["filePath"].get<std::string>());
	//スプライトのセッティング
	mPosition = json.value("position", mPosition);
	setBordarSize(json.value("borderSize", 0.0f));
	setSpriteSize(XMFLOAT2(json.value("width", 1.0f), json.value("height", 1.0f)));
	setRotation(json.value("rotation", 0.0f));
}


void SpriteComponent::endProcess()
{
	//Gameからスプライトを削除
	mOwner.getScene().removeSprite(this);
	mOwner.getScene().getGame().getAssetManager().deleteMemory(mCBIndex, mCBSize);
	mOwner.getScene().getGame().getAssetManager().deleteHeap(mHeapIndex, mHeapSize);
}

void SpriteComponent::create(const std::string filename)
{
	if(mCBSize == 0 && mHeapSize == 0) {
		//コンスタントバッファ、ディスクリプタヒープ用のインデックスを取得
		mCBSize = 256 * (1 + mNumSprites); //spriteConstantBuf + textureの数
		mHeapSize = 2 + mNumSprites;
		mCBIndex = mAssetManager.getCBEndIndex(mCBSize);
		mHeapIndex = mAssetManager.getHeapEndIndex(mHeapSize);

		//Sprite用の各Viewを取得
		SpriteData spriteData = mAssetManager.getSpriteData();
		mVertexBufView = spriteData.VertexBufView;
		mIndexBufView = spriteData.IndexBufView;

	}
	//テクスチャを取得
	mTextureSize = mAssetManager.createTextureAndGetSize(filename);
	mTextureBuf = mAssetManager.getShaderResource(filename);

	//コンスタントバッファの初期化
	Cb3.windowSize = XMFLOAT2(
		(float)Graphic::ClientWidth,
		(float)Graphic::ClientHeight
	);
	Cb3.spriteSize = mSpriteSize;
	Cb3.textureSize = mTextureSize;
	Cb3.bordarSize = mBordarSize;
	memcpy(mGraphic.getConstantData() + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//ディスクリプタヒープにViewを作る
	int heapIndex = mHeapIndex;
	mGraphic.createConstantBufferView(mCBIndex, 256, heapIndex, 1); heapIndex += 2;
	mGraphic.createShaderResourceView(mTextureBuf, heapIndex);

	mTextureFilePath = filename;
}

void SpriteComponent::loadFileAndCreate(const std::string& structName)
{
	//ファイルの読み込み
	std::ifstream spriteFile("assets\\data\\spriteData.json");
	nlohmann::json spriteJson;
	spriteFile >> spriteJson;

	//構造体が存在しない場合、作成する
	if (spriteJson.find(structName) == spriteJson.end()) {
		spriteJson[structName] = {
			{"filePath", mTextureFilePath},
			{"x", 0.0f},
			{"y", 0.0f},
			{"width", 100.0f},
			{"height", 100.0f},
			{"borderSize", 0.0f},
			{"rotation", 0.0f}
		};
		std::ofstream outFile("assets\\data\\spriteData.json");
		outFile << spriteJson.dump(4);
	}

	//スプライトの作成
	auto filePath = spriteJson[structName].value("filePath", "");
	if ( filePath != "")
		create(spriteJson[structName].value("filePath", mTextureFilePath));
	//スプライトのセッティング
	setPosition(XMFLOAT3(spriteJson[structName].value("x", 0.0f), spriteJson[structName].value("y", 0.0f), mPosition.z));
	setBordarSize(spriteJson[structName].value("borderSize", 0.0f));
	setSpriteSize(XMFLOAT2(spriteJson[structName].value("width", 1.0f), spriteJson[structName].value("height", 1.0f)));
	setRotation(spriteJson[structName].value("rotation", 0.0f));
}

void SpriteComponent::draw()
{
	//コンスタントバッファの更新
	//ワールドマトリックス
	XMMATRIX world = XMMatrixIdentity()
		* XMMatrixTranslation(-mSpriteSize.x * 0.5f, -mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixRotationZ(mRotation)
		* XMMatrixTranslation(mSpriteSize.x * 0.5f, mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixScaling(mScale.x, mScale.y, 1.0f)
		* XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z)
		;
	Cb3.world = world;
	Cb3.spriteSize = mSpriteSize;	//スプライトサイズ
	Cb3.bordarSize = mBordarSize;	//ボーダーサイズ
	//コンスタントバッファへコピー
	memcpy(mGraphic.getConstantData() + mCBIndex, &Cb3, sizeof(SpriteConstBuf));

	//頂点をセット
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	auto hCbvTbvHeap = mGraphic.getHeapHandle();
	UINT CbvTbvSize = mGraphic.getCbvTbvIncSize();
	hCbvTbvHeap.ptr += (mHeapIndex + mGraphic.getBackBufIdx()) * CbvTbvSize;

	mCommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
	hCbvTbvHeap = mGraphic.getHeapHandle();
	hCbvTbvHeap.ptr += (mHeapIndex + 2) * CbvTbvSize;
	mCommandList->SetGraphicsRootDescriptorTable(1, hCbvTbvHeap);
	//描画。インデックスを使用
	mCommandList->IASetIndexBuffer(&mIndexBufView);
	mCommandList->DrawIndexedInstanced(mAssetManager.getSpriteIndicesSize(), 1, 0, 0, 0);
	
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

void SpriteComponent::movePosition(const XMFLOAT2& diff)
{
	mPosition.x += diff.x;
	mPosition.y += diff.y;
}

void SpriteComponent::setPosX(float xPos)
{
	mPosition.x = xPos;
}

void SpriteComponent::setPosY(float yPos)
{
	mPosition.y = yPos;
}

void SpriteComponent::setPosZ(float zPos)
{
	mPosition.z = zPos;
}

//デバッグ用
#ifdef _DEBUG
void SpriteComponent::activateControll(const std::string& structName)
{
	mActiveControll = true;
	mStructName = structName;
}
#endif