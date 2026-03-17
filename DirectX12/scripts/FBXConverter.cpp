#include "FBXConverter.h"
#include <cassert>
#include <algorithm>
#include <fstream>

FBXConverter::FBXConverter() {
	//初期化
	Idx0 = 0;
	Idx1 = 1;
	Idx2 = 2;

	NumParts = 0;
	Parts = nullptr;
}

void FBXConverter::fbxToTxt(const char* fbxFilename, const char* txtFilename, float fx, float fy, float fz, int idx0, int idx1, int idx2)
{
	//FbxManager作成
	FbxManager* fbx_manager = fbxsdk::FbxManager::Create();
	assert(fbx_manager != nullptr);

	//FbxImporter作成
	FbxImporter* fbx_importer = FbxImporter::Create(fbx_manager, "");
	assert(fbx_importer != nullptr);
	//fbxファイル名をインポーターにセット
	bool result = fbx_importer->Initialize(fbxFilename);
	assert(result == true);

	//FbxScene作成
	FbxScene* fbx_scene = FbxScene::Create(fbx_manager, "");
	assert(fbx_scene != nullptr);
	//sceneにデータをインポート
	result = fbx_importer->Import(fbx_scene);
	assert(result == true);
	fbx_importer->Destroy();

	//コンバータを用意
	FbxGeometryConverter converter(fbx_manager);
	//メッシュに使われているマテリアル単位でメッシュを分割
	converter.SplitMeshesPerMaterial(fbx_scene, true);
	//ポリゴンを三角形にする(pLegacyをtrueにするとうまくいったなぜ？)
	converter.Triangulate(fbx_scene, true, true);

	//マテリアルの読み込み
	int numMaterials = fbx_scene->GetSrcObjectCount<FbxSurfaceMaterial>();
	for (int i = 0; i < numMaterials; i++) {
		LoadMaterial(fbx_scene->GetSrcObject<FbxSurfaceMaterial>(i));
	}

	//分割したメッシュをパーツごとに読み込む
	Idx0 = idx0; Idx1 = idx1; Idx2 = idx2;
	NumParts = fbx_scene->GetSrcObjectCount<FbxMesh>();
	Parts = new PARTS[NumParts];
	for (int k = 0; k < NumParts; k++) {
		auto* mesh = fbx_scene->GetSrcObject<FbxMesh>(k);
		LoadPosAndNorm(mesh, k);
		LoadTexcoods(mesh, k);
		LoadIndices(mesh, k);
		LoadMaterialName(mesh, k);
	}
	//ポインターの破棄
	fbx_scene->Destroy();
	fbx_manager->Destroy();

	//テキスト情報として出力
	CreateTextConbineVertex(txtFilename, fx, fy, fz);
	delete[] Parts;
}

//マテリアル読み込み
void FBXConverter::LoadMaterial(FbxSurfaceMaterial* material) {
	MATERIAL tmpMaterial;

	FbxDouble3 colors;
	FbxProperty prop;

	//ランバートの場合
	{
		//アンビエント
		prop = material->FindProperty(FbxSurfaceMaterial::sAmbient);
		if (prop.IsValid()) {
			colors = prop.Get<FbxDouble3>();
		}
		else {
			colors = FbxDouble3(0.0, 0.0, 0.0);
		}
		tmpMaterial.materials.emplace_back((float)colors[0]);
		tmpMaterial.materials.emplace_back((float)colors[1]);
		tmpMaterial.materials.emplace_back((float)colors[2]);
		tmpMaterial.materials.emplace_back(0.0f);
		//ディフーズ
		prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (prop.IsValid()) {
			colors = prop.Get<FbxDouble3>();
		}
		else {
			colors = FbxDouble3(1.0, 1.0, 1.0);
		}
		tmpMaterial.materials.emplace_back((float)colors[0]);
		tmpMaterial.materials.emplace_back((float)colors[1]);
		tmpMaterial.materials.emplace_back((float)colors[2]);
		tmpMaterial.materials.emplace_back(1.0f);
	}
	//フォンの場合
	{
		//スペキュラー
		prop = material->FindProperty(FbxSurfaceMaterial::sSpecular);
		if (prop.IsValid()) {
			colors = prop.Get<FbxDouble3>();
		}
		else {
			colors = FbxDouble3(0.0, 0.0, 0.0);
		}
		tmpMaterial.materials.emplace_back((float)colors[0]);
		tmpMaterial.materials.emplace_back((float)colors[1]);
		tmpMaterial.materials.emplace_back((float)colors[2]);

		double shininess;
		prop = material->FindProperty(FbxSurfaceMaterial::sShininess);
		if (prop.IsValid()) {
			shininess = prop.Get<FbxDouble>();
		}
		else {
			shininess = 0.0f;
		}
		tmpMaterial.materials.emplace_back((float)shininess);
	}

	//テクスチャ名の取得(シングル対応)
	FbxFileTexture* texture = nullptr;

	//Diffuseプロパティを取得
	prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	int texture_num = prop.GetSrcObjectCount<FbxFileTexture>();
	if (texture_num > 0) {
		//propからFbxFileTextureを取得
		texture = prop.GetSrcObject<FbxFileTexture>(0);
	}
	else {
		//FbxLayeredTextureからFbxFileTextureを取得
		int layer_num = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (layer_num > 0) {
			texture = prop.GetSrcObject<FbxFileTexture>(0);
		}
	}

	//テクスチャがなければマテリアルのみ登録
	if (texture == nullptr) {
		MaterialMap[material->GetName()] = tmpMaterial;
		return;
	}

	//パス付ファイル名を取得
	std::string pathName = texture->GetRelativeFileName();
	// '/'を'\\'に置き換え
	std::replace(pathName.begin(), pathName.end(), '/', '\\');
	//最後の'\\'の位置を検索
	size_t lastSlashPos = pathName.rfind('\\');
	if (lastSlashPos != std::string::npos) {
		//パスのないファイル名のみを取り出す
		tmpMaterial.textureFilename = pathName.c_str() + lastSlashPos + 1;
	}
	else {
		tmpMaterial.textureFilename = pathName;
	}

	//マップに登録
	MaterialMap[material->GetName()] = tmpMaterial;

}

//メッシュ読み込み
void FBXConverter::LoadPosAndNorm(FbxMesh* mesh, int k) {
	FbxVector4* positions = mesh->GetControlPoints();

	FbxNode* node = mesh->GetNode();
	
	//頂点座標の取得
	FbxVector4 translation = Math::scale(node->LclTranslation.Get(), FbxVector4(0.01, 0.01, 0.01, 1.0)); //100倍した値で返ってくるので、0.01倍する
	FbxVector4 preTrans = translation;
	FbxVector4 rotation = Math::scale(node->LclRotation.Get(), FbxVector4(PI / 180, PI / 180, PI / 180, 1.0)); //度数で返ってくるのでラジアンへ変換
	FbxVector4 scale = Math::scale(node->LclScaling.Get(), FbxVector4(0.01, 0.01, 0.01, 1.0)); //%の値が返ってくるので、0.01倍する

	//インデックスの取得
	int* indices = mesh->GetPolygonVertices();
	//頂点数の取得
	int polygonVertexCount = mesh->GetPolygonVertexCount();
	//メンバのfloat配列確保
	Parts[k].Positions.reserve(polygonVertexCount * 3);

	//頂点座標をメンバに保存
	for (int i = 0; i < polygonVertexCount; i++) {
		int index = indices[i];
		FbxVector4 position = positions[index];

		//座標変換
		position = Math::translate(Math::rotateZ(Math::rotateY(Math::rotateX(Math::scale(positions[index], scale), rotation[0]), rotation[1]), rotation[2]), translation);

		Parts[k].Positions.emplace_back((float)(position[Idx0]));
		Parts[k].Positions.emplace_back((float)(position[Idx1]));
		Parts[k].Positions.emplace_back((float)(-1 * position[Idx2]));	//左手系に変換
	}


	FbxArray<FbxVector4> normals;
	mesh->GetPolygonVertexNormals(normals);
	Parts[k].Normals.reserve(normals.Size() * 3);
	for (int i = 0; i < normals.Size(); i++) {
		//座標変換
		normals[i] = Math::normalize(Math::translate(Math::rotateZ(Math::rotateY(Math::rotateX(Math::scale(normals[i], scale), rotation[0]), rotation[1]), rotation[2]), translation));

		Parts[k].Normals.emplace_back((float)(normals[i][Idx0]));
		Parts[k].Normals.emplace_back((float)(normals[i][Idx1]));
		Parts[k].Normals.emplace_back((float)(-1 * normals[i][Idx2]));	//左手系に変換
	}

}

void FBXConverter::LoadNormals(FbxMesh* mesh, int k) {
	FbxArray<FbxVector4> normals;
	mesh->GetPolygonVertexNormals(normals);
	Parts[k].Normals.reserve(normals.Size() * 3);
	for (int i = 0; i < normals.Size(); i++) {
		Parts[k].Normals.emplace_back((float)normals[i][Idx0]);
		Parts[k].Normals.emplace_back((float)normals[i][Idx1]);
		Parts[k].Normals.emplace_back((float)normals[i][Idx2]);
	}
}

void FBXConverter::LoadTexcoods(FbxMesh* mesh, int k) {
	//UVSetの名前リストを取得
	FbxStringList uvset_names;
	mesh->GetUVSetNames(uvset_names);

	//UVSetの名前からUVSetを取得
	//マルチテクスチャ非対応のため、最初の名前を参照
	FbxArray<FbxVector2> texcoords;
	mesh->GetPolygonVertexUVs(uvset_names.GetStringAt(0), texcoords);
	Parts[k].Texcoords.reserve(texcoords.Size() * 2);
	for (int i = 0; i < texcoords.Size(); i++) {
		Parts[k].Texcoords.emplace_back((float)texcoords[i][0]);
		Parts[k].Texcoords.emplace_back((float)(1.0 - texcoords[i][1]));
	}
}

void FBXConverter::LoadIndices(FbxMesh* mesh, int k)
{
	//ポリゴン数の取得
	int polygonCount = mesh->GetPolygonCount();
	//配列確保
	Parts[k].Indices.reserve(polygonCount * 3);
	//ポリゴンの数だけ連番として保存する
	for (int i = 0; i < polygonCount; i++)
	{
		Parts[k].Indices.emplace_back(i * 3 + 0);
		Parts[k].Indices.emplace_back(i * 3 + 1);
		Parts[k].Indices.emplace_back(i * 3 + 2);
	}
}

void FBXConverter::LoadMaterialName(FbxMesh* mesh, int k) {
	//マテリアルが無ければ終了
	if (mesh->GetElementMaterialCount() == 0) {
		Parts[k].MaterialName = "";
		return;
	}

	//Mesh側のマテリアル情報を取得
	FbxLayerElementMaterial* material = mesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);
	FbxSurfaceMaterial* surface_material = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);
	if (surface_material != nullptr) {
		Parts[k].MaterialName = surface_material->GetName();
	}
	else {
		Parts[k].MaterialName = "";
	}
}


//③テキストデータとして出力-----------------------------------------------------------------
//position,normal,texcoordまとめて１頂点として出力
void FBXConverter::CreateTextConbineVertex(const char* txtFilename, float fx, float fy, float fz)
{
	std::ofstream ofs(txtFilename);

	ofs << NumParts << '\n';

	for (int k = 0; k < NumParts; k++) {
		//頂点
		{
			size_t numVertices = Parts[k].Positions.size() / 3;
			ofs << "vertices " << numVertices << "\n";
			size_t i = 0;
			size_t j = 0;//Texcoordsのインデックス
			while (i < Parts[k].Positions.size()) {
				ofs << fx * Parts[k].Positions[i] << " ";
				ofs << fy * Parts[k].Positions[i + 1] << " ";
				ofs << fz * Parts[k].Positions[i + 2] << " ";

				if (fx >= 0)ofs << Parts[k].Normals[i] << " ";
				else ofs << -Parts[k].Normals[i] << " ";
				if (fy >= 0)ofs << Parts[k].Normals[i + 1] << " ";
				else ofs << -Parts[k].Normals[i + 1] << " ";
				if (fz >= 0)ofs << Parts[k].Normals[i + 2] << " ";
				else ofs << -Parts[k].Normals[i + 2] << " ";
				i += 3;

				ofs << Parts[k].Texcoords[j + 0] << " ";
				ofs << Parts[k].Texcoords[j + 1] << "\n";
				j += 2;
			}
		}
#ifdef USE_INDEX
		//インデックス
		{
			size_t numIndices = Parts[k].Indices.size();//要素数がインデックス数。頂点系とは違います。
			ofs << "indices " << numIndices << "\n";
			size_t i = 0;
			while (i < numIndices) {
				ofs << Parts[k].Indices[i++] << " ";
				ofs << Parts[k].Indices[i++] << " ";
				ofs << Parts[k].Indices[i++] << "\n";
			}
		}
#endif
		//マテリアル
		{
			ofs << "material\n";
			//メッシュ側のマテリアル名でマテリアルマップから引っ張ってくる
			float* d = MaterialMap[Parts[k].MaterialName].materials.data();
			if (d != nullptr) {
				ofs << d[0] << " " << d[1] << " " << d[2] << " " << d[3] << "\n";
				ofs << d[4] << " " << d[5] << " " << d[6] << " " << d[7] << "\n";
				ofs << d[8] << " " << d[9] << " " << d[10] << " " << d[11] << "\n";
			}
		}
		//テクスチャ名
		{
			ofs << "texture ";

			//メッシュ側のマテリアル名でマテリアルマップから引っ張ってくる
			std::string& textureFilename
				= MaterialMap[Parts[k].MaterialName].textureFilename;

			//テクスチャ名がなかったら、white.pngにする
			if (textureFilename == "") {
				ofs << "assets/picture/white.png\n";
				continue;
			}

			//これから引数のtxtFilenameからパス名だけ取り出します
			std::string pathName = txtFilename;
			//'/'を'\\'に置き換え
			std::replace(pathName.begin(), pathName.end(), '/', '\\');
			//最後の'\\'の位置を検索
			size_t lastSlashPos = pathName.rfind('\\');
			if (lastSlashPos != std::string::npos) {
				//パス名のみにします。
				pathName.erase(lastSlashPos + 1);
				//パス名とテクスチャファイル名をドッキングして出力します
				ofs << pathName + textureFilename << "\n";
			}
			else {
				ofs << textureFilename << "\n";
			}
		}
	}
}

//posision,normal,texcoord別々に出力
void FBXConverter::CreateText(const char* txtFilename, float fx, float fy, float fz)
{
	std::ofstream ofs(txtFilename);

	ofs << NumParts << '\n';

	for (int k = 0; k < NumParts; k++)
	{
		{
			size_t numVertices = Parts[k].Positions.size() / 3;
			ofs << "positions " << numVertices << "\n";
			size_t i = 0;
			while (i < Parts[k].Positions.size()) {
				ofs << fx * Parts[k].Positions[i++] << " ";
				ofs << fy * Parts[k].Positions[i++] << " ";
				ofs << fz * Parts[k].Positions[i++] << "\n";//右手座標で表示。手前がマイナス。
			}
		}
		{
			size_t numNormals = Parts[k].Normals.size() / 3;
			ofs << "normals " << numNormals << "\n";
			size_t i = 0;
			while (i < Parts[k].Normals.size()) {
				if (fx >= 0)ofs << Parts[k].Normals[i++] << " ";
				else ofs << -Parts[k].Normals[i++] << " ";
				if (fy >= 0)ofs << Parts[k].Normals[i++] << " ";
				else ofs << -Parts[k].Normals[i++] << " ";
				if (fz >= 0)ofs << Parts[k].Normals[i++] << "\n";
				else ofs << -Parts[k].Normals[i++] << "\n";
			}
		}
		{
			size_t numVertices = Parts[k].Texcoords.size() / 2;
			ofs << "texcoords " << numVertices << "\n";
			size_t i = 0;
			while (i < Parts[k].Texcoords.size()) {
				ofs << Parts[k].Texcoords[i++] << " ";
				ofs << Parts[k].Texcoords[i++] << "\n";
			}
		}
		{
			size_t numIndices = Parts[k].Indices.size();//要素数がインデックス数。頂点系とは違います。
			ofs << "indices " << numIndices << "\n";
			size_t i = 0;
			while (i < Parts[k].Indices.size()) {
				ofs << Parts[k].Indices[i++] << " ";
				ofs << Parts[k].Indices[i++] << " ";
				ofs << Parts[k].Indices[i++] << "\n";
			}
		}
		{
			ofs << "material\n";
			float* d = MaterialMap[Parts[k].MaterialName].materials.data();
			ofs << d[0] << " " << d[1] << " " << d[2] << "\n";
			ofs << d[3] << " " << d[4] << " " << d[5] << "\n";
			ofs << d[6] << " " << d[7] << " " << d[8] << "\n";
			ofs << d[9] << "\n";
		}
		{
			ofs << "texture ";

			//メッシュ側のマテリアル名でマテリアルマップから引っ張ってくる
			if (MaterialMap[Parts[k].MaterialName].textureFilename == "") {
				ofs << "assets/picture/white.png";
				return;
			}

			//これからtxtFilenameからパス名だけ取り出します
			std::string pathName = txtFilename;
			//'/'を'\\'に置き換え
			std::replace(pathName.begin(), pathName.end(), '/', '\\');
			//最後の'\\'の位置をを検索
			size_t lastSlashPos = pathName.rfind('\\');
			if (lastSlashPos != std::string::npos) {
				//パス名のみにします。
				pathName.erase(lastSlashPos + 1);
				//パス名とテクスチャファイル名をドッキングして出力します
				ofs << pathName + MaterialMap[Parts[k].MaterialName].textureFilename << "\n";
			}
			else {
				ofs << MaterialMap[Parts[k].MaterialName].textureFilename << "\n";
			}
		}
	}
}
