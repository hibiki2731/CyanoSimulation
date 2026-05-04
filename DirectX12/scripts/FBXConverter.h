#pragma once
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "Math.h"

struct PARTS {
	std::vector<float> Positions;
	std::vector<float> Normals;
	std::vector<float> Texcoords;
	std::vector<uint16_t> Indices;
	std::vector<float> materials;
	std::string texturePath;
};

class FBXConverter
{
public:
	FBXConverter();
	void fbxToJson(
		const char* fbxFilename, const char* jsonFilename,
		float fx = 1, float fy = 1, float fz = 1,
		int idx0 = 0, int idx1 = 1, int idx2 = 2);

private:
	//マテリアルのLoad
	void LoadMaterial(FbxSurfaceMaterial* material);
	struct MATERIAL {
		std::vector<float> materials;
		std::string        textureFilename;
	};
	std::unordered_map<std::string, MATERIAL> MaterialMap;

	//メッシュのLoad
	int Idx0, Idx1, Idx2;
	void LoadPosAndNorm(FbxMesh* mesh, int k);
	void LoadNormals(FbxMesh* mesh, int k);
	void LoadTexcoods(FbxMesh* mesh, int k);
	void LoadIndices(FbxMesh* mesh, int k);
	void LoadPartsMaterial(FbxMesh* mesh, const std::string& filaPath, int k);
	int NumParts;
	std::vector<PARTS> mParts;

	//テキストデータの出力
	void CreateJson(const std::string& jsonFileName, float fx, float fy, float fz);
};

