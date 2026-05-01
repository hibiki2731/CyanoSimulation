#pragma once
#include <string>
#include "Actor.h"

class Resource : public Actor
{
public:
	Resource(class DungeonScene& scene, const std::string& resourceID,const std::string& meshID, float x, float y, int index);
	DECLARE_CLASS_NAME(Resource)

	void collect();

	//getter
	const std::string& getResourceID() { return mResourceID; }
	const std::string& getResourceName() { return mName; }
	int getAmount() { return mAcquiredAmount; }

private:
	std::string mResourceID;
	std::string mName;
	class DungeonScene& mDungeonScene;
	class ItemManager& mItemManager;

	int mIndex;
	int mLife;
	int mAcquiredAmount;

};

