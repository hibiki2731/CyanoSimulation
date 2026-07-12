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
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "SpriteCBSuballocation.h"

SpriteComponent::SpriteComponent(Actor& owner, float zDepth) 
	: Component(owner),
	mGraphic(owner.getScene().getGame().getGraphic()),
	mConstantBuffer(mGraphic.getConstantBuffer()),
	mDescriptorHeap(mGraphic.getDescriptorHeap()),
	mAssetManager(owner.getScene().getGame().getAssetManager())
{
	mPosition = { 0.0f, 0.0f, zDepth};
	mScale = { 1.0f, 1.0f };
	mSpriteSize = { 100.0f, 100.0f };
	mBordarSize = 0.0f;
	mRotation = 0.0f;

	mCommandList = mGraphic.getCommandList();
	mOwner.getScene().addSprite(this);

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

	mDescriptorHeap.deleteRange(*mDescRange);
	mConstantBuffer.deleteSuballocation(*mSpriteCBSubData);
}

void SpriteComponent::create(const std::string filename)
{
	if(!isInitialized) {
		//ディスクリプタヒープのスロットを確保
		mDescRange = mDescriptorHeap.allocate(3);

		//コンスタントバッファのサブアロケーションを確保
		mSpriteCBSubData = mConstantBuffer.createSuballocation<SpriteCBSuballocation>(sizeof(SpriteCBSuballocationData));

		//Sprite用の各Viewを取得
		SpriteData spriteData = mAssetManager.getSpriteData();
		mVertexBufView = spriteData.VertexBufView;
		mIndexBufView = spriteData.IndexBufView;

		isInitialized = true;

	}
	//テクスチャを取得
	mTextureSize = mAssetManager.createTextureAndGetSize(filename);
	mTextureBuf = mAssetManager.getShaderResource(filename);

	//コンスタントバッファの初期化
	mSpriteCBSubData->updateWindowSize(XMFLOAT2(
		(float)Graphic::ClientWidth,
		(float)Graphic::ClientHeight
	));
	mSpriteCBSubData->updateSpriteSize(mSpriteSize);
	mSpriteCBSubData->updateTextureSize(mTextureSize);
	mSpriteCBSubData->updateBordarSize(mBordarSize);
	//データをGPUメモリにコピー
	mSpriteCBSubData->applyChanges(0);
	mSpriteCBSubData->applyChanges(1);

	//ディスクリプタヒープにViewを作る
	mDescriptorHeap.addCBVFrameCounts(*mSpriteCBSubData, mDescRange->getIndex(0), 1);
	mDescriptorHeap.addSRV(*mTextureBuf, mDescRange->getIndex(2));

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
	mSpriteCBSubData->updateWorld(XMMatrixIdentity()
		* XMMatrixTranslation(-mSpriteSize.x * 0.5f, -mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixRotationZ(mRotation)
		* XMMatrixTranslation(mSpriteSize.x * 0.5f, mSpriteSize.y * 0.5f, 0.0f)
		* XMMatrixScaling(mScale.x, mScale.y, 1.0f)
		* XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z));
	mSpriteCBSubData->updateSpriteSize(mSpriteSize);	//スプライトサイズ
	mSpriteCBSubData->updateBordarSize(mBordarSize);	//ボーダーサイズ
	//コンスタントバッファへコピー
	mSpriteCBSubData->applyChanges(mGraphic.getBackBufIdx());

	//頂点をセット
	mCommandList->IASetVertexBuffers(0, 1, &mVertexBufView);

	//ディスクリプタヒープをディスクリプタテーブルにセット
	mCommandList->SetGraphicsRootDescriptorTable(0, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(mGraphic.getBackBufIdx())));
	mCommandList->SetGraphicsRootDescriptorTable(1, mDescriptorHeap.getGPUHandle(mDescRange->getIndex(2)));
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
