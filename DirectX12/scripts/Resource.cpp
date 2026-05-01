#include "Resource.h"
#include "MeshComponent.h"
#include "Game.h"
#include "json.hpp"
#include "ItemManager.h"
#include "DungeonScene.h"
#include "AssetManager.h"
#include <fstream>

Resource::Resource(DungeonScene& scene, const std::string& resourceID, const std::string& meshID, float x, float y, int index) : 
	Actor(scene, x, y), mDungeonScene(scene), mItemManager(mScene.getGame().getItemManager())
{
	auto mesh = std::make_unique<MeshComponent>(*this);
	mesh->create(meshID);
	addComponent(std::move(mesh));

	//リソースデータの読み込み
	auto& resourceData = mItemManager.getResourceData(resourceID);
	mLife = resourceData.life;
	mAcquiredAmount = resourceData.acquiredAmount;
	mIndex = index;
	mName = resourceData.name;
	mResourceID = resourceID;
	//リソース
	mDungeonScene.setTileDataAt(mIndex, TileType::RESOURCE);
}

void Resource::collect()
{
	mScene.getGame().getItemManager().addResource(mResourceID, mAcquiredAmount);

	mLife--;
	if (mLife <= 0) {
		//リソースの除去
		mDungeonScene.deleteResourceFromIndex(mIndex);
		mState = Actor::State::Dead;
		//マップデータの更新
		mDungeonScene.setTileDataAt(mIndex, TileType::FLOOR);

		//新たなリソースを出現させる
		mDungeonScene.spawnResource();

		mDungeonScene.updateMiniMapPos();
	}
}
