#include "Object.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Game.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "myJson.h"

//メッシュのみを生成するためのコンストラクタ
Object::Object(Scene& scene, const std::string& name, const std::string& meshID, float x, float z) 
	: Actor(scene, x, z),
	mName(name)
{
	mPosition = XMFLOAT3(x, 0, z);

	//メッシュコンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	//所有権をActorに渡す
	addComponent(std::move(mesh));

}

//オブジェクトIDから生成するためのコンストラクタ
Object::Object(Scene& scene, const std::string& objectID)
	:Actor(scene),
	mName(objectID)
{
	//jsonの取得
	nlohmann::json& objJson = scene.getGame().getAssetManager().getObjectJson()[scene.getName()];

	//オブジェクトIDが存在するかどうか判定し、存在する場合JSONから各情報を取得する
	if (objJson.contains(objectID)) {
		//アクター情報を取得
		loadActorData(objJson[objectID]);
	}

	//編集用配列へ追加
#ifdef _DEBUG
	mScene.addDebugObject(this);
#endif


}

//アクターの終了処理
void Object::endProcessActor()
{
	//編集用配列から除去
#ifdef _DEBUG
	mScene.removeDebugObject(this);
#endif
}

//ラベル名とそれに対応する型名を登録
void Object::addComponentLabel(const std::string& labelName, const std::string& componentName)
{
	//すでに同じ名前のラベルが登録されていたら無視する
	if (mComponentLabels.contains(labelName)) {
		return;
	}

	mComponentLabels[labelName].componentName = componentName;
	mComponentLabels[labelName].pComponent = nullptr;
}

void Object::loadActorData(nlohmann::json& json)
{
	//アクター情報がなければ何も実行しない
	if (json.empty()) return;

	//アクターデータの取得
	mPosition = json.value("position", XMFLOAT3(0.0f, 0.0f, 0.0f));
	mRotation = json.value("rotation", XMFLOAT3(0.0f, 0.0f, 0.0f));
	mScale = json.value("scale", XMFLOAT3(1.0f, 1.0f, 1.0f));

	//コンポーネントの取得
	loadComponentData(json);

	//ラベルデータの取得
	loadLabelData(json);

}

void Object::loadComponentData(nlohmann::json& json)
{
	//コンポーネントの取得
	for (auto componentJson : json["components"]) {

		//コンポーネントの型名を取得
		std::string componentName = componentJson["name"];
		//スプライト
		if (componentName == "SpriteComponent") {
			auto sprite = std::make_unique<SpriteComponent>(*this);
			sprite->loadFromJson(componentJson);
			addComponent(std::move(sprite));
		}
		//テキスト
		else if (componentName == "TextComponent") {
			auto text = std::make_unique<TextComponent>(*this);
			text->loadFromJson(componentJson);
			addComponent(std::move(text));
		}
	}
}

void Object::loadLabelData(nlohmann::json& json)
{
	//ラベルがなければ終了
	if (json["label"].empty()) return;

	//JSONはラベル名をキーとして、IDと型名を要素とする構造体を配列として保持
	//IDはObjectの持つコンポーネント配列のindexを指す
	for (auto& [labelName, labelJson] : json["label"].items()) {
		ComponentLabel label;
		
		//IDから参照するコンポーネントを取得
		int id = labelJson.at("id").get<int>();
		if (id < 0) {
			label.pComponent = nullptr;
		}
		else {
			label.pComponent = mComponents[id].get();
		}
		//コンポーネントの型名を取得
		label.componentName = labelJson.at("componentName").get<std::string>();

		mComponentLabels[labelName] = label;
	}
}
