#include "MeshComponent.h"
#include<fstream>
#include<vector>
#include "Graphic.h"
#include "Actor.h"
#include "Scene.h"
#include "Game.h"
#include "myJson.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "MeshWorldCBSuballocation.h"
#include "MeshBaseCBSuballocation.h"
#include "MeshMaterialCBSuballocation.h"
#include "VertexBuffer.h"

MeshComponent::MeshComponent(Actor& owner, const std::shared_ptr<class MeshBaseCBSuballocation>& baseSuballocation, int updateOrder) 
	: Component(owner, updateOrder),
	mGraphic(owner.getScene().getGame().getGraphic()),
	mDescriptorHeap(mGraphic.getDescriptorHeap()),
	mConstantBuffer(mGraphic.getConstantBuffer())
{
	mCommandList = mGraphic.getCommandList();
	mOwner.getScene().addMesh(this);
	mBaseSuballocation = baseSuballocation;
	mMeshID = "NONE";
	isInitialized = false;
}

void MeshComponent::loadFromJson(const nlohmann::json& json)
{
	std::string meshID = json.value("meshID", "GRASS");
	create(meshID);
}

void MeshComponent::endProcess()
{
	//Gameからメッシュを削除
	mOwner.getScene().removeMesh(this);

	//ディスクリプタヒープのスロットを解放
	mDescriptorHeap.deleteRange(*mDescRange);
	//コンスタントバッファのサブアロケーションを解放
	mConstantBuffer.deleteSuballocation(*mWorldSuballocation);
	for (auto& part : Parts) mConstantBuffer.deleteSuballocation(*part.MaterialSuballocation);

}

void MeshComponent::create(const std::string& meshID)
{
	MeshData* meshData = mOwner.getScene().getGame().getAssetManager().getMeshData(meshID);
	if (meshData == nullptr) return;

	mMeshID = meshID;
	create(meshData);
}

void MeshComponent::create(const MeshData * meshData)
{	
	if (meshData == nullptr) return;

	//メッシュパーツ数を読み込み、メモリを確保
	NumParts = meshData->NumParts;
	Parts.resize(NumParts);

	//ディスクリプタヒープのスロットを確保
	NumAllPartsDescriptors = NumParts * NumDescriptors;
	mDescRange = mDescriptorHeap.allocate(NumSlots(NumAllPartsDescriptors * Graphic::FrameCount)); //全パーツのディスクリプタ * フレーム分

	//フラッシュ用バッファの初期化
	mWorldSuballocation = mConstantBuffer.createSuballocation<MeshWorldCBSuballocation>(sizeof(MeshWorldCBSuballocationData));
	mWorldSuballocation->updateFlashColor(XMFLOAT3(1.0f, 1.0f, 1.0f));	//白く光る
	mWorldSuballocation->updateFlashIntensity(0.0f);				//最初は光らない

	//パーツごとに各バッファ情報を取得
	for (int k = 0; k < NumParts; k++) {
		//頂点バッファ
		{
			auto& vertexBuf = meshData->VertexBuf[k];
			Parts[k].NumVertices = vertexBuf.getNumVertices();
			Parts[k].VertexBufView = vertexBuf.getView();
		}
		//マテリアル用コンスタントバッファ
		{
			Parts[k].MaterialSuballocation = mConstantBuffer.createSuballocation<MeshMaterialCBSuballocation>(sizeof(MeshMaterialCBSuballocationData));
			Parts[k].MaterialSuballocation->updateAmbient(meshData->Material[k * 3]);
			Parts[k].MaterialSuballocation->updateDiffuse(meshData->Material[k * 3 + 1]);
			Parts[k].MaterialSuballocation->updateSpecular(meshData->Material[k * 3 + 2]);
			Parts[k].MaterialSuballocation->applyChanges();

		}
		{
			Parts[k].TextureBuf = mOwner.getScene().getGame().getAssetManager().getShaderResource(meshData->TextureName[k]);
		}

		//ビューの作成
		//Baseデータ
		mDescriptorHeap.addCBVFrameCounts(*mBaseSuballocation.lock(), mDescRange->getIndex(k), NumAllPartsDescriptors);
		//Worldデータ
		auto& ptr2 = *mWorldSuballocation;
		mDescriptorHeap.addCBVFrameCounts(*mWorldSuballocation, mDescRange->getIndex(k + 1), NumAllPartsDescriptors);
		//Materialデータ
		mDescriptorHeap.addCBVFrameCounts(*Parts[k].MaterialSuballocation, mDescRange->getIndex(k + 2), NumAllPartsDescriptors);
		//テクスチャデータ
		mDescriptorHeap.addSRVFrameCounts(*Parts[k].TextureBuf, mDescRange->getIndex(k + 3), NumAllPartsDescriptors);

	}

	//スケールを設定
	mOwner.setScale(meshData->Scale);

	isInitialized = true;

}

void MeshComponent::draw()
{

	if (!isInitialized) return;

	//ワールドマトリックス
	mWorldSuballocation->updateWorld(XMMatrixIdentity()
		* XMMatrixScaling(mOwner.getScale().x, mOwner.getScale().y, mOwner.getScale().z)
		* XMMatrixRotationX(mOwner.getRotation().x)
		* XMMatrixRotationY(mOwner.getRotation().y)
		* XMMatrixRotationZ(mOwner.getRotation().z)
		* XMMatrixTranslation(mOwner.getPosition().x, mOwner.getPosition().y, mOwner.getPosition().z))
		;

	//CPU上のヴァーチャルアドレスへコピー
	mWorldSuballocation->applyChanges(mGraphic.getBackBufIdx());

	int backBufIdx = mGraphic.getBackBufIdx();
	//パーツごとに描画
	for (int k = 0; k < NumParts; ++k)
	{
		//頂点をセット
		mCommandList->IASetVertexBuffers(0, 1, &Parts[k].VertexBufView);

		//ディスクリプタヒープをディスクリプタテーブルにセット
		auto hGPU = mDescriptorHeap.getGPUHandle(mDescRange->getIndex(k * NumDescriptors + backBufIdx * NumAllPartsDescriptors));
		mCommandList->SetGraphicsRootDescriptorTable(0, hGPU);

		//描画。インデックスを使用しない
		mCommandList->DrawInstanced(Parts[k].NumVertices, 1, 0, 0);

	}
}

void MeshComponent::updateFlashIntensity(float intensity)
{
	mWorldSuballocation->updateFlashIntensity(intensity);
}

const std::string& MeshComponent::getMeshID()
{
	return mMeshID;
}
