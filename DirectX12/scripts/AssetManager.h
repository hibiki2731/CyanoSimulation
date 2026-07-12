#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include "json.hpp"
#include "Definition.h"
#include "directx/d3d12.h"

class Graphic;
class VertexBuffer;
class IndexBuffer;

struct MeshData {
	int NumParts;
	//頂点バッファ
	std::vector<VertexBuffer> VertexBuf;
	//マテリアルデータ
	std::vector<XMFLOAT4> Material;
    std::vector<std::string> TextureName;
	//スケール
	XMFLOAT3 Scale;
};

struct SpriteData {
	D3D12_VERTEX_BUFFER_VIEW VertexBufView;
	D3D12_INDEX_BUFFER_VIEW IndexBufView;
};

enum class MeshName {
	ROCK_WALL,
	ROCK_FLOOR,
	COUNT,
};

class AssetManager
{
public:

	AssetManager(Graphic& graphic);
	~AssetManager();

	//getter
	MeshData* getMeshData(const std::string& meshID); 
	SpriteData getSpriteData();
	XMFLOAT2 createTextureAndGetSize(const std::string& filePath);
	ID3D12Resource* getShaderResource(const std::string& textureName);
	UINT getSpriteVerticesSize();
	UINT getSpriteIndicesSize();
	nlohmann::json& getObjectJson() { return mObjectJson; }
	nlohmann::json& getSceneJson() { return mSceneJson; }

	void loadObjectJson();

private:
	//メッシュデータをjsonから読み込むための構造体
	struct MeshFileData {
		std::string filePath;
		std::vector<float> scale;
	};

	Graphic& mGraphic;
	std::unordered_map<std::string, std::unique_ptr<MeshData>> mLoadData;
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> mTextureData; //テクスチャデータのキャッシュ

	//スプライト用
	std::unique_ptr<VertexBuffer> mSpriteVertexBuf;
	std::unique_ptr<IndexBuffer> mSpriteIndexBuf;
	std::unordered_map<std::string, XMFLOAT2> mTextureSizeData;

	//json
	nlohmann::json mEnemyJson;
	nlohmann::json mObjectJson;
	nlohmann::json mSceneJson;

	void createMeshData(const std::string& meshID, const MeshFileData& meshFileData);
	void createSpriteBuffers();
	void loadJson();
};

