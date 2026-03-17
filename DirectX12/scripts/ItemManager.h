#pragma once
#include <unordered_map>
#include <string>

struct ItemData {
	std::string id;
	std::string name;
	std::string category;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int value;
};

struct WeaponData {
	std::string id;
	std::string name;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int power;
};

struct ArmerData {
	std::string id;
	std::string name;
	std::string category;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int defence;
};

struct ExplorerData {
	std::string id;
	std::string name;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	std::string category;
	int value;
};

class ItemManager
{
public:
	ItemManager();

	void addResource(std::string id, int num);
	void subResource(std::string id, int num);

	int getResourceNum(std::string id);
	const std::unordered_map<std::string, size_t>& getResourceData();
	const ItemData& getItemData(std::string id);
	const WeaponData& getWeaponData(std::string id);
	const ArmerData& getArmerData(std::string id);
	const ExplorerData& getExplorerData(std::string id);

	static const ItemData EmptyItem;
	static const WeaponData EmptyWeapon;
	static const ArmerData EmptyArmer;
	static const ExplorerData EmptyExplorer;
private:
	std::unordered_map<std::string, size_t> mResourceData;
	std::unordered_map<std::string, ItemData> mItemData;
	std::unordered_map<std::string, WeaponData> mWeaponData;
	std::unordered_map<std::string, ArmerData> mArmerData;
	std::unordered_map<std::string, ExplorerData> mExplorerData;

};

