#pragma once
#include <map>
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

	AssetManager(Graphic* graphic);
	~AssetManager();


	//getter
	int getCBEndIndex(int size);//必要なサイズを引数に取る
	int getHeapEndIndex(int size); //必要なサイズを引数に取る
	MeshData* getMeshData(MeshName objectName); 
	SpriteData getSpriteData();
	XMFLOAT2 createTextureAndGetSize(const std::string& filePath);
	ID3D12Resource* getShaderResource(const std::string& textureName);
	UINT getSpriteVerticesSize();
	UINT getSpriteIndicesSize();

	void deleteMemory(int index, int size);
	void deleteHeap(int index, int size);


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

	Graphic* mGraphic;
	std::map<MeshName, std::unique_ptr<MeshData>> mLoadData;
	std::map<std::string, ComPtr<ID3D12Resource>> mTextureData; //テクスチャデータのキャッシュ
	std::vector<ClearedMemory> mClearedMemory; //解放されたメモリ
	std::vector<ClearedHeap> mClearedHeap; //解放されたメモリ

	//スプライト用
	ComPtr<ID3D12Resource> mSpriteVertexBuf;
	D3D12_VERTEX_BUFFER_VIEW mSpriteVertexBufView;
	ComPtr<ID3D12Resource> mSpriteIndexBuf;
	D3D12_INDEX_BUFFER_VIEW mSpriteIndexBufView;
	std::map<std::string, XMFLOAT2> mTextureSizeData;

	void createMesh(MeshName objectName);
	void createSpriteBuffers();
};

