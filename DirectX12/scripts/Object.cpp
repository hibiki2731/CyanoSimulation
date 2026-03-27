#include "Object.h"
#include "MeshComponent.h"

Object::Object(Game* game, const std::string& meshID, float x, float y) : Actor(game, x, y)
{
	mPosition = XMFLOAT3(x, 0, y);

	//メッシュコンポーネントの作成
	auto mesh = std::make_unique<MeshComponent>(this);
	mesh->create(meshID);
	//所有権をActorに渡す
	addComponent(std::move(mesh));

}
