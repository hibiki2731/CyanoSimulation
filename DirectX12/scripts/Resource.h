#pragma once
#include <string>
#include "Actor.h"

class Resource : public Actor
{
public:
	Resource(class DungeonScene& scene, const std::string& meshID,const std::string& resourceID, float x, float y);

	void updateActor() override;
	void inputActor() override;

private:
	const std::string& mResourceID;
};

