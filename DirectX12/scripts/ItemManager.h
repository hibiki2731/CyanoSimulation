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
	bool inPossession;
};

struct ArmerData {
	std::string id;
	std::string name;
	std::string category;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	int defence;
	bool inPossession;
};

struct ExplorerData {
	std::string id;
	std::string name;
	std::vector<std::string> costResourceID;
	std::vector<int> price;
	std::string category;
	int value;
	bool inPossession;
};

class ItemManager
{
public:
	ItemManager();

	void loadItemData();

	void addResource(const std::string& id, int num);
	void subResource(const std::string& id, int num);

	//getter
	int getResourceNum(std::string id);
	const std::unordered_map<std::string, size_t>& getResourceData();
	const ItemData& getItemData(const std::string& id);
	const std::unordered_map<std::string, ItemData>& getItemData();
	const WeaponData& getWeaponData(const std::string& id);
	const std::unordered_map<std::string, WeaponData>& getWeaponData();
	const ArmerData& getArmerData(std::string id);
	const std::unordered_map<std::string, ArmerData>& getArmerData();
	const ExplorerData& getExplorerData(const std::string& id);
	const std::unordered_map<std::string, ExplorerData>& getExplorerData();

	//setter
	void setWeaopnPossession(std::string id, bool possession);
	void setArmerPossession(std::string id, bool possession);
	void setToolPossession(std::string id, bool possession);

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

