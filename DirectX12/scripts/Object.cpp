#include "Object.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Game.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"
#include "FireParticleComponent.h"

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

Object::Object(Scene& scene, const std::string& name)
	:Actor(scene),
	mName(name)
{
}
