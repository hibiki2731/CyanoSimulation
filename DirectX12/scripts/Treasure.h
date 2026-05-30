#pragma once
#include "Actor.h"

class Treasure : public Actor
{
public:
	Treasure(Scene& scene, int x, int y, const std::string& direction, const std::string& category, const std::string& itemID);

	bool equal(int x, int y) const;

	void open();

	//クラスの名前を取得
	const std::string getClassName() const override {
		return "Treasure";
	}


private:
	const int mX;
	const int mY;
	const std::string mDirection;
	const std::string mCategory;
	const std::string mItemID;

	bool mIsOpen = false;
	
	static const std::string TreasureMeshID;
	static const std::string TreasureOpenMeshID;
};

