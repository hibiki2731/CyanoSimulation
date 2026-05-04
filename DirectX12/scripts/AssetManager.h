#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include "Graphic.h"
#include "json.hpp"
#include "magic_enum.hpp"

struct MeshData {
	int NumParts;
	//頂点バッファ
	std::vector<UINT> NumVertices;
	std::vector<ComPtr<ID3D12Resource>> VertexBuf;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VertexBufView;
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
	GRASS,
	SLIME,
	NURIKABE,
	COUNT,
};

class AssetManager
{
public:

	AssetManager(Graphic& graphic);

	//getter
	int getCBEndIndex(int size);//必要なサイズを引数に取る
	int getHeapEndIndex(int size); //必要なサイズを引数に取る
	MeshData* getMeshData(const std::string& meshID); 
	SpriteData getSpriteData();
	XMFLOAT2 createTextureAndGetSize(const std::string& filePath);
	ID3D12Resource* getShaderResource(const std::string& textureName);
	UINT getSpriteVerticesSize();
	UINT getSpriteIndicesSize();
	nlohmann::json& getEnemyJson() { return mEnemyJson; }
	nlohmann::json& getObjectJson() { return mObjectJson; }
	nlohmann::json& getSceneJson() { return mSceneJson; }

	void deleteMemory(int index, int size);
	void deleteHeap(int index, int size);

	void loadObjectJson();

private:
	//解放されたメモリやヒープを管理するための構造体
	struct ClearedMemory {
		int index;
		int size;
	};

	struct ClearedHeap {
		int index;
		int size;
	};

	//メッシュデータをjsonから読み込むための構造体
	struct MeshFileData {
		std::string filePath;
		std::vector<float> scale;
	};

	int mCBEndIndex; //コンスタントバッファの使用済みメモリの最後尾インデックス
	int mHeapEndIndex; //ディスクリプタヒープの最後尾インデックス

	Graphic& mGraphic;
	std::unordered_map<std::string, std::unique_ptr<MeshData>> mLoadData;
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> mTextureData; //テクスチャデータのキャッシュ
	std::map<int, int> mClearedMemory; //解放されたメモリ
	std::map<int, int> mClearedHeap; //解放されたメモリ

	//スプライト用
	ComPtr<ID3D12Resource> mSpriteVertexBuf;
	D3D12_VERTEX_BUFFER_VIEW mSpriteVertexBufView;
	ComPtr<ID3D12Resource> mSpriteIndexBuf;
	D3D12_INDEX_BUFFER_VIEW mSpriteIndexBufView;
	std::unordered_map<std::string, XMFLOAT2> mTextureSizeData;


	//json
	nlohmann::json mEnemyJson;
	nlohmann::json mObjectJson;
	nlohmann::json mSceneJson;

	void createMeshData(const std::string& meshID, const MeshFileData& meshFileData);
	void createSpriteBuffers();
	void loadJson();
};

