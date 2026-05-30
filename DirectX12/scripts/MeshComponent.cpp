#include "MeshComponent.h"
#include<fstream>
#include<vector>
#include "Graphic.h"
#include "Actor.h"
#include "Scene.h"
#include "Game.h"
#include "myJson.h"

MeshComponent::MeshComponent(Actor& owner, int updateOrder) 
	: Component(owner, updateOrder),
	mGraphic(owner.getScene().getGame().getGraphic())
{
	mCommandList = mGraphic.getCommandList();
	mOwner.getScene().addMesh(this);
	CbvTbvSize = mGraphic.getCbvTbvIncSize();
	mMeshID = "NONE";
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
	mOwner.getScene().getGame().getAssetManager().deleteMemory(mCBIndex, mCBSize);
	mOwner.getScene().getGame().getAssetManager().deleteHeap(mHeapIndex, mHeapSize * 2);
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

	//コンスタントバッファのインデックスを取得
	mCBSize = (meshData->NumParts + 1) * 256;
	mCBIndex = mOwner.getScene().getGame().getAssetManager().getCBEndIndex(mCBSize);
	mHeapSize = NumDescriptors * meshData->NumParts;
	mHeapIndex = mOwner.getScene().getGame().getAssetManager().getHeapEndIndex(mHeapSize * 2); //二つ分Viewを作る必要がある

	//メッシュパーツ数を読み込み、メモリを確保
	NumParts = meshData->NumParts;
	Parts.resize(NumParts);

	//フラッシュ用バッファの初期化
	Cb1.flashColor = XMFLOAT3(1.0f, 1.0f, 1.0f);	//白く光る
	Cb1.flashIntensity = 0.0f;				//最初は光らない

	//パーツごとに各バッファ情報を取得
	for (int k = 0; k < NumParts; k++) {
		//頂点バッファ
		{
			Parts[k].NumVertices = meshData->NumVertices[k];
			Parts[k].VertexBufView = meshData->VertexBufView[k];
		}
		//マテリアル用コンスタントバッファ
		{
			Parts[k].Cb2.ambient = meshData->Material[k * 3];
			Parts[k].Cb2.diffuse = meshData->Material[k * 3 + 1];
			Parts[k].Cb2.specular = meshData->Material[k * 3 + 2];
		}
		{
			Parts[k].TextureBuf = mOwner.getScene().getGame().getAssetManager().getShaderResource(meshData->TextureName[k]);
		}
	}

	//コンスタントバッファへデータをコピー
	for (int i = 0; i < NumParts; i++) {
		memcpy(mGraphic.getConstantData(0) + mCBIndex + 256 * (i + 1), &Parts[i].Cb2, sizeof(MaterialConstBuf));
		memcpy(mGraphic.getConstantData(1) + mCBIndex + 256 * (i + 1), &Parts[i].Cb2, sizeof(MaterialConstBuf));

	}



	int heapIndex = mHeapIndex;
	//コンスタントバッファビューを作成
	for (int k = 0; k < NumParts; k++) {
		mGraphic.createBase3DBufferView(heapIndex, mHeapSize); heapIndex++;
		mGraphic.createConstantBufferView(mCBIndex, 256, heapIndex, mHeapSize); heapIndex++;
		mGraphic.createConstantBufferView(mCBIndex + 256 * (k + 1), 256, heapIndex, mHeapSize); heapIndex++;
		mGraphic.createShaderResourceView(Parts[k].TextureBuf, heapIndex); 
		mGraphic.createShaderResourceView(Parts[k].TextureBuf, heapIndex + mHeapSize); 
		heapIndex++;
	}

	//スケールを設定
	mOwner.setScale(meshData->Scale);

}

void MeshComponent::draw()
{
	//ワールドマトリックス
	XMMATRIX world = XMMatrixIdentity()
		* XMMatrixScaling(mOwner.getScale().x, mOwner.getScale().y, mOwner.getScale().z)
		* XMMatrixRotationX(mOwner.getRotation().x)
		* XMMatrixRotationY(mOwner.getRotation().y)
		* XMMatrixRotationZ(mOwner.getRotation().z)
		* XMMatrixTranslation(mOwner.getPosition().x, mOwner.getPosition().y, mOwner.getPosition().z)
		;
	Cb1.world = world;

	memcpy(mGraphic.getConstantData() + mCBIndex, &Cb1, sizeof(World3DConstBuf));

	auto hCbvTbvHeap = mGraphic.getHeapHandle();
	hCbvTbvHeap.ptr += CbvTbvSize * (mHeapIndex + mGraphic.getBackBufIdx() * mHeapSize);

	//パーツごとに描画
	for (int k = 0; k < NumParts; ++k)
	{
		//頂点をセット
		mCommandList->IASetVertexBuffers(0, 1, &Parts[k].VertexBufView);

		//ディスクリプタヒープをディスクリプタテーブルにセット
		mCommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
		hCbvTbvHeap.ptr += CbvTbvSize * NumDescriptors;

		//描画。インデックスを使用しない
		mCommandList->DrawInstanced(Parts[k].NumVertices, 1, 0, 0);

	}
}

void MeshComponent::updateFlashIntensity(float intensity)
{
	Cb1.flashIntensity = intensity;
}

const std::string& MeshComponent::getMeshID()
{
	return mMeshID;
}
