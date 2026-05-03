#include "Object.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Game.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"

#include "FireParticleComponent.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "myJson.h"

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

Object::Object(Scene& scene, const std::string& objectID)
	:Actor(scene),
	mName(objectID)
{
	//jsonの取得
	nlohmann::json& objJson = scene.getGame().getAssetManager().getObjectJson()[scene.getName()];

	//オブジェクトIDが存在するかどうか判定
	if (objJson.contains(objectID)) {
		//アクター情報を取得
		loadActorData(objJson[objectID]);
	}

	//編集用配列へ追加
#ifdef _DEBUG
	mScene.addDebugObject(this);
#endif


}

void Object::endProcessActor()
{
#ifdef _DEBUG
	mScene.removeDebugObject(this);
#endif
}

void Object::addComponentLabel(const std::string& labelName, const std::string& componentName)
{
	if (mComponentLabels.contains(labelName)) {
		return;
	}

	mComponentLabels[labelName].componentName = componentName;
	mComponentLabels[labelName].pComponent = nullptr;
}

void Object::loadActorData(nlohmann::json& json)
{
	if (json.empty()) return;

	//アクターデータの取得
	mPosition = json.value("position", XMFLOAT3(0.0f, 0.0f, 0.0f));
	mRotation = json.value("rotation", XMFLOAT3(0.0f, 0.0f, 0.0f));
	mScale = json.value("scale", XMFLOAT3(1.0f, 1.0f, 1.0f));

	//コンポーネントの取得
	loadComponentData(json);

	//ラベルデータの取得
	loadLabeData(json);

}

void Object::loadComponentData(nlohmann::json& json)
{
	//コンポーネントの取得
	for (auto componentJson : json["components"]) {

		std::string componentName = componentJson["name"];
		//メッシュ
		if (componentName == "MeshComponent") {
			auto mesh = std::make_unique<MeshComponent>(*this);
			mesh->loadFromJson(componentJson);
			addComponent(std::move(mesh));
		}
		//点光源
		if (componentName == "PointLightComponent") {
			//光源
			auto light = std::make_unique<PointLightComponent>(*this);
			light->loadFromJson(componentJson);
			addComponent(std::move(light));
		}
		//炎パーティクル
		else if (componentName == "FireParticleComponent") {
			auto fire = std::make_unique<FireParticleComponent>(*this);
			fire->loadFromJson(componentJson);
			addComponent(std::move(fire));
		}
		else if (componentName == "SpriteComponent") {
			auto sprite = std::make_unique<SpriteComponent>(*this);
			sprite->loadFromJson(componentJson);
			addComponent(std::move(sprite));
		}
		else if (componentName == "TextComponent") {
			auto text = std::make_unique<TextComponent>(*this);
			text->loadFromJson(componentJson);
			addComponent(std::move(text));
		}
	}
}

void Object::loadLabeData(nlohmann::json& json)
{
	//ラベルがなければ終了
	if (json["label"].empty()) return;

	for (auto& [labelName, labelJson] : json["label"].items()) {
		ComponentLabel label;
		int id = labelJson.at("id").get<int>();
		if (id < 0) {
			label.pComponent = nullptr;
		}
		else {
			label.pComponent = mComponents[id].get();
		}
		label.componentName = labelJson.at("componentName").get<std::string>();

		mComponentLabels[labelName] = label;
	}
}
