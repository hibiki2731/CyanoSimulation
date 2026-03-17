#pragma once
#include <fbxsdk.h>
#include <vector>
#include <map>
#include <string>
#include "Math.h"

class FBXConverter
{
public:
	FBXConverter();
	void fbxToTxt(
		const char* fbxFilename, const char* txtFilename,
		float fx = 1, float fy = 1, float fz = 1,
		int idx0 = 0, int idx1 = 1, int idx2 = 2);

private:
	//マテリアルのLoad
	void LoadMaterial(FbxSurfaceMaterial* material);
	struct MATERIAL {
		std::vector<float> materials;
		std::string        textureFilename;
	};
	std::map<std::string, MATERIAL> MaterialMap;

	//メッシュのLoad
	int Idx0, Idx1, Idx2;
	void LoadPosAndNorm(FbxMesh* mesh, int k);
	void LoadNormals(FbxMesh* mesh, int k);
	void LoadTexcoods(FbxMesh* mesh, int k);
	void LoadIndices(FbxMesh* mesh, int k);
	void LoadMaterialName(FbxMesh* mesh, int k);
	int NumParts;
	struct PARTS {
		std::vector<float> Positions;
		std::vector<float> Normals;
		std::vector<float> Texcoords;
		std::vector<uint16_t> Indices;
		std::string MaterialName;
	}* Parts;

	//テキストデータの出力
	void CreateTextConbineVertex(const char* txtFilename, float fx, float fy, float fz);
	void CreateText(const char* txtFileName, float fx, float fy, float fz);
};

