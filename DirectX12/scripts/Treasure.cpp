#include "Treasure.h"
#include "Definition.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "Game.h"
#include "PlayerManager.h"

const std::string Treasure::TreasureMeshID = "TREASURE";
const std::string Treasure::TreasureOpenMeshID = "TREASURE_OPEN";

Treasure::Treasure(Scene& scene, int x, int y, const std::string& direction, const std::string& category, const std::string& itemID)
	: Actor(scene, static_cast<float>(x) * MAPTIPSIZE, static_cast<float>(y) * MAPTIPSIZE)
	, mX(x)
	, mY(y)
	, mDirection(direction)
	, mCategory(category)
	, mItemID(itemID)
{
	//3Dモデルの生成
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(TreasureMeshID);
	addComponent(std::move(mesh));

	mIsOpen = false;

	if (direction == "UP") {
		setYRot(0);
	}
	else if (direction == "RIGHT") {
		setYRot(XM_PIDIV2);
	}
	else if (direction == "DOWN") {
		setYRot(XM_PI);
	}
	else if (direction == "LEFT") {
		setYRot(-XM_PIDIV2);
	}
}

bool Treasure::equal(int x, int y) const
{
	return (mX == x && mY == y);
}

void Treasure::open()
{

	if (mIsOpen) return;

	//メッシュの変更
	removeComponent(mComponents[0].get()); //メッシュコンポーネントの削除

	auto openMesh = std::make_unique<MeshComponent>(*this);
	openMesh->create(TreasureOpenMeshID);
	addComponent(std::move(openMesh));

	//アイテムの取得
	//アイテムのカテゴリーを判定
	if (mCategory == "TOOL") {
		//ツールの取得
		mScene.getGame().getPlayerManager().addExplorer(mItemID);
		mScene.getGame().getPlayerManager().applyToolParamater();
	}
	else if (mCategory == "ARMER") {
		//防具の取得
		mScene.getGame().getPlayerManager().addArmer(mItemID);
	}
	else if (mCategory == "WEAPON") {
		//武器の取得
		mScene.getGame().getPlayerManager().addWeapon(mItemID);
	}

	mIsOpen = true;
}
