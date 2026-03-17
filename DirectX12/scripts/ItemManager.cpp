#include "ItemManager.h"
#include "json.hpp"
#include <fstream>

//空のアイテムデータ
const ItemData ItemManager::EmptyItem = {
	"",
	"",
	"",
	{""},
	{0},
	0
};

const WeaponData ItemManager::EmptyWeapon = {
	"",
	"",
	{""},
	{0},
	0
};

const ArmerData ItemManager::EmptyArmer{
	"",
	"",
	"",
	{""},
	{0},
	0
};

const ExplorerData ItemManager::EmptyExplorer{
	"",
	"",
	{""},
	{0}
};

ItemManager::ItemManager()
{


	std::fstream file("assets/data/ItemData.json");
	assert(!file.fail());

	nlohmann::json json;
	file >> json;
	//保存してあるリソース数を読み込む
	for (auto& resourceJson : json["Resource"]) {
		mResourceData[resourceJson["id"]] = resourceJson["num"];
	}
	//アイテムデータを読み込む
	for (auto& itemJson : json["Item"]) {
		ItemData item;
		item.id = itemJson["id"];
		item.name = itemJson["name"];
		item.category = itemJson["category"];
		item.costResourceID = itemJson["costResourceID"];
		item.value = itemJson["value"];
		item.price = itemJson["price"].get<std::vector<int>>();
		mItemData[itemJson["id"]] = item;
	}
	//武器データを読み込む
	for (auto& weaponJson : json["Weapon"]) {
		WeaponData weapon;
		weapon.id = weaponJson["id"];
		weapon.name = weaponJson["name"];
		weapon.costResourceID = weaponJson["costResourceID"];
		weapon.price = weaponJson["price"].get<std::vector<int>>();
		weapon.power = weaponJson["power"];
		mWeaponData[weaponJson["id"]] = weapon;
	}
	//防具データを読み込む
	for (auto& armerJson : json["Armer"]) {
		ArmerData armer;
		armer.id = armerJson["id"];
		armer.name = armerJson["name"];
		armer.category = armerJson["category"];
		armer.costResourceID = armerJson["costResourceID"];
		armer.price = armerJson["price"].get<std::vector<int>>();
		armer.defence = armerJson["defence"];
		mArmerData[armerJson["id"]] = armer;
	}
	//探索道具データを読み込む
	for (auto& explorerJson : json["Explorer"]) {
		ExplorerData explorer;
		explorer.id = explorerJson["id"];
		explorer.name = explorerJson["name"];
		explorer.costResourceID = explorerJson["costResourceID"];
		explorer.price = explorerJson["price"].get<std::vector<int>>();
		explorer.category = explorerJson["category"];
		explorer.value = explorerJson["value"];
		mExplorerData[explorerJson["id"]] = explorer;
	}
}

void ItemManager::addResource(std::string id, int num)
{
	auto iter = mResourceData.find(id);
	//idが存在しない場合
	if (iter == mResourceData.end()) return;
	//存在する場合
	iter->second += static_cast<size_t>(num);
}

void ItemManager::subResource(std::string id, int num)
{
	auto iter = mResourceData.find(id);
	//idが存在しない場合
	if (iter == mResourceData.end()) return;
	//存在する場合
	int value = static_cast<int>(iter->second - num);
	if (value < 0) return; //数が負の数になる場合、0で止める

	iter->second = static_cast<size_t>(value);
}

int ItemManager::getResourceNum(std::string id) {
	auto iter = mResourceData.find(id);
	if (iter != mResourceData.end())
		//idが存在する場合
		return static_cast<int>(iter->second);
	else
		//idが存在しない場合
		return 0;
	
}

const std::unordered_map<std::string, size_t>& ItemManager::getResourceData()
{
	return mResourceData;
}

const ItemData& ItemManager::getItemData(std::string id)
{
	auto iter = mItemData.find(id);
	if (iter != mItemData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyItem;
}

const WeaponData& ItemManager::getWeaponData(std::string id)
{
	auto iter = mWeaponData.find(id);
	if (iter != mWeaponData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyWeapon;
}

const ArmerData& ItemManager::getArmerData(std::string id)
{
	auto iter = mArmerData.find(id);
	if (iter != mArmerData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyArmer;

}

const ExplorerData& ItemManager::getExplorerData(std::string id)
{
	auto iter = mExplorerData.find(id);
	if (iter != mExplorerData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyExplorer;

}
