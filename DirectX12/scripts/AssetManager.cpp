#include "pch.h"
#include "AssetManager.h"
#include "Graphic.h"
#include <string>
#include <fstream>
#include "FBXConverter.h"
#include "json.hpp"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

static std::vector<float> spriteVertices = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.3333f, 0.0f, 0.3333f,
	 0.3333f, 0.0f, 0.3333f, 0.0f,
	 0.3333f, 0.3333f, 0.3333f, 0.3333f,
	 0.6666f, 0.0f, 0.6666f, 0.0f,
	 0.6666f, 0.3333f, 0.6666f, 0.3333f,
	 1.0f, 0.0f, 1.0f, 0.0f,
	 1.0f, 0.3333f, 1.0f, 0.3333f,
	 0.0f,  0.6666f, 0.0f, 0.6666f,
	 0.0f,  1.0f, 0.0f, 1.0f,
	 0.3333f,  0.6666f, 0.3333f, 0.6666f,
	 0.3333f,  1.0f, 0.3333f, 1.0f,
	 0.6666f,  0.6666f, 0.6666f, 0.6666f,
	 0.6666f,  1.0f, 0.6666f, 1.0f,
	 1.0f,  0.6666f, 1.0f, 0.6666f,
	 1.0f,  1.0f, 1.0f, 1.0f,
};

static std::vector<UINT16> spriteIndices = {
	0, 1, 2,
	2, 1, 3,
	2, 3, 4,
	4, 3, 5,
	4, 5, 6,
	6, 5, 7,
	1, 8, 3,
	3, 8, 10,
	3, 10, 5,
	5, 10, 12,
	5, 12, 7,
	7, 12, 14,
	8, 9, 10,
	10, 9, 11,
	10, 11, 12,
	12, 11, 13,
	12, 13, 14,
	14, 13, 15,
};

static const int NumElementsPerVertex = 8;

AssetManager::AssetManager(Graphic& graphic)
	: mGraphic(graphic)
{
	//Base3DDataが先頭のメモリを使用する
	createSpriteBuffers();

	std::fstream file("assets/data/MeshData.json");
	nlohmann::json json;
	file >> json;

#ifdef _DEBUG
	FBXConverter fbxConverter;
#endif

	//全メッシュの読み込みを最初に行う
	for (const auto& [key, value] : json.items()) {
		MeshFileData meshFileData;

		meshFileData.filePath = value["jsonPath"].get<std::string>();
		meshFileData.scale = value.value("scale", std::vector<float>{1.0f, 1.0f, 1.0f});
#ifdef _DEBUG
		//FBXファイルの変換
		fbxConverter.fbxToJson(value["fbxPath"].get<std::string>().c_str(), meshFileData.filePath.c_str() , 1.0f, 1.0f, 1.0f, 0, 1, 2); //横、縦、奥行
#endif

		createMeshData(key, meshFileData);
	}

	loadJson();
}

AssetManager::~AssetManager() = default;

void AssetManager::createMeshData(const std::string& meshID, const MeshFileData& meshFileData)
{
	if (mLoadData.contains(meshID)) return; //すでに読み込まれたオブジェクトはスルー

	//ファイルを読み込む
	std::ifstream meshFile(meshFileData.filePath);
	assert(!meshFile.fail());
	//jsonへ読み込み
	nlohmann::json json;
	meshFile >> json;

	auto meshData = std::make_unique<MeshData>();

	//メッシュパーツ数を読み込み、メモリを確保
	int numParts = json["numParts"].get<int>();
	meshData->NumParts = numParts;
	meshData->VertexBuf.reserve(numParts);
	meshData->Material.resize(3 * numParts); 
	meshData->TextureName.resize(numParts);
	meshData->Scale = XMFLOAT3(meshFileData.scale[0], meshFileData.scale[1], meshFileData.scale[2]);

	int k = -1; //パーツのインデックス

	//パーツごとのデータを読み込む
	for (auto partsJson : json["parts"]) {
		k++;
		//頂点バッファ
		{
			//jsonから生データをvector配列に読み込む
			std::vector<float> positions = partsJson["position"].get<std::vector<float>>();
			std::vector<float> normals = partsJson["normal"].get<std::vector<float>>();
			std::vector<float> texcoords = partsJson["texcoord"].get<std::vector<float>>();
			int numVertices = positions.size() / 3;

			//一つのvector配列に格納
			std::vector<float> vertices(numVertices * NumElementsPerVertex);
			for (int i = 0; i < numVertices; i++) {
				vertices[i * NumElementsPerVertex] = positions[i * 3];
				vertices[i * NumElementsPerVertex + 1] = positions[i * 3 + 1];
				vertices[i * NumElementsPerVertex + 2] = positions[i * 3 + 2];
				vertices[i * NumElementsPerVertex + 3] = normals[i * 3];
				vertices[i * NumElementsPerVertex + 4] = normals[i * 3 + 1];
				vertices[i * NumElementsPerVertex + 5] = normals[i * 3 + 2];
				vertices[i * NumElementsPerVertex + 6] = texcoords[i * 2];
				vertices[i * NumElementsPerVertex + 7] = texcoords[i * 2 + 1];
			}

			//頂点バッファをつくる
			VertexBufferDescription desc = {numVertices, NumElementsPerVertex};
			meshData->VertexBuf.emplace_back(*mGraphic.getDevice(), desc, vertices);

		}
		//マテリアル
		{
			std::vector<float> material = partsJson["material"].get<std::vector<float>>();

			meshData->Material[k * 3] = XMFLOAT4(material[0], material[1], material[2], material[3]);
			meshData->Material[k * 3 + 1] = XMFLOAT4(material[4], material[5], material[6], material[7]);
			meshData->Material[k * 3 + 2] = XMFLOAT4(material[8], material[9], material[10], material[11]);
		}
		//テクスチャバッファ
		{
			//ファイル名を読み込む

			std::string texturePath = partsJson["texturePath"].get<std::string>();

			auto iter = mTextureData.find(texturePath);
			if (iter != mTextureData.end()) {
				//すでに読み込まれたテクスチャはスルー
				meshData->TextureName[k] = texturePath;
				continue;
			}

			ComPtr<ID3D12Resource> textureBuf;
			HRESULT hr = mGraphic.createShaderResource(texturePath, textureBuf);
			assert(SUCCEEDED(hr));
			meshData->TextureName[k] = texturePath;
			mTextureData[texturePath] = std::move(textureBuf);

		}
	}

	mLoadData[meshID] = std::move(meshData);
}

XMFLOAT2 AssetManager::createTextureAndGetSize(const std::string& filePath)
{
	XMFLOAT2 size;

	auto iter = mTextureData.find(filePath);
	if (iter != mTextureData.end()) {
		//すでに読み込まれたテクスチャはそのまま返す
		size = mTextureSizeData[filePath];
	}
	else {
		ComPtr<ID3D12Resource> textureBuf;
		size = mGraphic.createShaderResourceGetSize(filePath, textureBuf);
		mTextureSizeData[filePath] = size;
		mTextureData[filePath] = std::move(textureBuf);
	}

	return size;
}

ID3D12Resource* AssetManager::getShaderResource(const std::string& textureName)
{
	ID3D12Resource* texture;
	auto iter = mTextureData.find(textureName);
	if (iter != mTextureData.end()) {
		//すでに読み込まれたテクスチャはそのまま返す
		texture = mTextureData[textureName].Get();
	}
	else {
		ComPtr<ID3D12Resource> textureBuf;
		mTextureSizeData[textureName] = mGraphic.createShaderResourceGetSize(textureName, textureBuf);
		texture = textureBuf.Get();
		mTextureData[textureName] = std::move(textureBuf);
	}

	return texture;

}

UINT AssetManager::getSpriteVerticesSize()
{
	return std::size(spriteVertices);
}

UINT AssetManager::getSpriteIndicesSize() {
	return std::size(spriteIndices);
}

MeshData* AssetManager::getMeshData(const std::string& meshID)
{
	auto iter = mLoadData.find(meshID);
	if (iter != mLoadData.end()) {
		return iter->second.get();
	}
	else {
		std::fstream file("assets/data/MeshData.json");
		nlohmann::json json;
		file >> json;

		//メッシュデータに目的のIDが存在するか調べる
		for (const auto& [key, value] : json.items()) {
			if (meshID != key) continue;

			MeshFileData meshFileData;
			meshFileData.filePath = value["filePath"].get<std::string>();
			meshFileData.scale = value.value("scale", std::vector<float>{ 1.0f, 1.0f, 1.0f });
			createMeshData(meshID, meshFileData);
			return mLoadData[meshID].get();
		}
	}

	return nullptr;
}

SpriteData AssetManager::getSpriteData()
{
	SpriteData spriteData = {
		mSpriteVertexBuf->getView(),
		mSpriteIndexBuf->getView()
	};

	return spriteData;
}

void AssetManager::createSpriteBuffers()
{
	{
		//頂点バッファの作成
		VertexBufferDescription desc = { spriteVertices.size(), 4};
		mSpriteVertexBuf = std::make_unique<VertexBuffer>(*mGraphic.getDevice(), desc, spriteVertices);
	
		//インデックスバッファの作成
		mSpriteIndexBuf = std::make_unique<IndexBuffer>(*mGraphic.getDevice(), sizeof(spriteIndices), spriteIndices);
	}
}

void AssetManager::loadJson()
{
	//オブジェクトデータファイルの読み込み
	std::ifstream objectDataFile("assets\\data\\objectData.json");
	assert(!objectDataFile.fail());
	objectDataFile >> mObjectJson;

	//敵パラメータファイルの読み込み
	std::ifstream sceneDataFile("assets\\data\\sceneData.json");
	assert(!sceneDataFile.fail());

	sceneDataFile >> mSceneJson;


}

void AssetManager::loadObjectJson()
{
	//オブジェクトデータファイルの読み込み
	std::ifstream objectDataFile("assets\\data\\objectData.json");
	assert(!objectDataFile.fail());
	objectDataFile >> mObjectJson;
}


