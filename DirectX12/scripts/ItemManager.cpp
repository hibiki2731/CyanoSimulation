#include "ItemManager.h"
#include "json.hpp"
#include <fstream>

//空のアイテムデータ
const ItemData ItemManager::EmptyItem = {
	"",
	"なし",
	"EMPTY",
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

const ResourceData ItemManager::EmptyResource{
	"",
	"",
	0
};

ItemManager::ItemManager()
{
}

void ItemManager::loadItemData()
{
	std::fstream file("assets/data/ItemData.json");
	assert(!file.fail());

	nlohmann::json json;
	file >> json;
	//保存してあるリソース数を読み込む
	for (auto& resourceJson : json["Resource"]) {
		ResourceData resource;
		resource.id = resourceJson["id"];
		resource.name = resourceJson["name"];
		resource.num = resourceJson["num"];
		mResourceData[resourceJson["id"]] = std::move(resource);
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
		item.effectText = itemJson["effectText"].get<std::string>();
		item.iconFilePath = itemJson["iconFilePath"].get<std::string>();
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
		weapon.inPossession = weaponJson["inPossession"];
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
		armer.inPossession = armerJson["inPossession"];
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
		explorer.value = explorerJson.value("value", 0);
		explorer.inPossession = explorerJson["inPossession"];
		mExplorerData[explorerJson["id"]] = explorer;
	}
}

void ItemManager::addResource(const std::string& id, int num)
{
	auto iter = mResourceData.find(id);
	//idが存在しない場合
	if (iter == mResourceData.end()) return;
	//存在する場合
	iter->second.num += static_cast<size_t>(num);
}

void ItemManager::subResource(const std::string& id, int num)
{
	auto iter = mResourceData.find(id);
	//idが存在しない場合
	if (iter == mResourceData.end()) return;
	//存在する場合
	int value = static_cast<int>(iter->second.num - num);
	if (value < 0) return; //数が負の数になる場合、0で止める

	iter->second.num = static_cast<size_t>(value);
}

int ItemManager::getResourceNum(std::string id) {
	auto iter = mResourceData.find(id);
	if (iter != mResourceData.end())
		//idが存在する場合
		return static_cast<int>(iter->second.num);
	else
		//idが存在しない場合
		return 0;
	
}

const ResourceData& ItemManager::getResourceData(const std::string& id)
{
	auto iter = mResourceData.find(id);
	if (iter != mResourceData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyResource;
}

const std::unordered_map<std::string, ResourceData>& ItemManager::getResourceData()
{
	return mResourceData;
}

const ItemData& ItemManager::getItemData(const std::string& id)
{
	auto iter = mItemData.find(id);
	if (iter != mItemData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyItem;
}

const std::unordered_map<std::string, ItemData>& ItemManager::getItemData()
{
	return mItemData;
}

const WeaponData& ItemManager::getWeaponData(const std::string& id)
{
	auto iter = mWeaponData.find(id);
	if (iter != mWeaponData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyWeapon;
}

const std::unordered_map<std::string, WeaponData>& ItemManager::getWeaponData()
{
	return mWeaponData;
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

const std::unordered_map<std::string, ArmerData>& ItemManager::getArmerData()
{
	return mArmerData;
}

const ExplorerData& ItemManager::getExplorerData(const std::string& id)
{
	auto iter = mExplorerData.find(id);
	if (iter != mExplorerData.end())
		//idが存在する場合
		return iter->second;
	else
		//idが存在しない場合、空データを渡す
		return EmptyExplorer;

}

const std::unordered_map<std::string, ExplorerData>& ItemManager::getExplorerData()
{
	return mExplorerData;
}

void ItemManager::setWeaopnPossession(std::string id, bool possession)
{
	auto iter = mWeaponData.find(id);
	if (iter != mWeaponData.end())
		iter->second.inPossession = possession;
}

void ItemManager::setArmerPossession(std::string id, bool possession)
{
	auto iter = mArmerData.find(id);
	if (iter != mArmerData.end())
		iter->second.inPossession = possession;
}

void ItemManager::setToolPossession(std::string id, bool possession)
{
	auto iter = mExplorerData.find(id);
	if (iter != mExplorerData.end())
		iter->second.inPossession = possession;
}
