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
	std::string description;
	std::string iconFilePath;
};

struct WeaponData {
	std::string id;
	std::string name;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int power;
	bool inPossession;
	std::string description;
};

struct ArmerData {
	std::string id;
	std::string name;
	std::string category;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int defence;
	bool inPossession;
	std::string description;
};

struct ExplorerData {
	std::string id;
	std::string name;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	std::string category;
	int value;
	bool inPossession;
	std::string description;
	std::string valueID;
};

struct ResourceData {
	std::string id;
	std::string name;
	size_t num;
	int life;
	int yield;
	int defaultYield;
};

class ItemManager
{
public:
	ItemManager();

	void loadItemData();

	void addResource(const std::string& id, int num);
	void subResource(const std::string& id, int num);
	void addResourceYield(const std::string& id, int num);

	//getter
	int getResourceNum(const std::string& id);
	const ResourceData& getResourceData(const std::string& id);
	const std::unordered_map<std::string, ResourceData>& getResourceData();
	const ItemData& getItemData(const std::string& id);
	const std::unordered_map<std::string, ItemData>& getItemData();
	const WeaponData& getWeaponData(const std::string& id);
	const std::unordered_map<std::string, WeaponData>& getWeaponData();
	const ArmerData& getArmerData(std::string id);
	const std::unordered_map<std::string, ArmerData>& getArmerData();
	const ExplorerData& getExplorerData(const std::string& id);
	const std::unordered_map<std::string, ExplorerData>& getExplorerData();

	//setter
	void setWeaopnPossession(const std::string& id, bool possession);
	void setArmerPossession(const std::string& id, bool possession);
	void setToolPossession(const std::string& id, bool possession);

	static const ItemData EmptyItem;
	static const WeaponData EmptyWeapon;
	static const ArmerData EmptyArmer;
	static const ExplorerData EmptyExplorer;
	static const ResourceData EmptyResource;
private:
	std::unordered_map<std::string, ResourceData> mResourceData;
	std::unordered_map<std::string, ItemData> mItemData;
	std::unordered_map<std::string, WeaponData> mWeaponData;
	std::unordered_map<std::string, ArmerData> mArmerData;
	std::unordered_map<std::string, ExplorerData> mExplorerData;
};

