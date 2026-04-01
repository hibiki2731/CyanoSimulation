#include "Object.h"
#include "MeshComponent.h"

Object::Object(Scene& scene, const std::string& meshID, float x, float y) : Actor(scene, x, y)
{
	mPosition = XMFLOAT3(x, 0, y);

	//メッシュコンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	//所有権をActorに渡す
	addComponent(std::move(mesh));

}
