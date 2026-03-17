#include "PlayerManager.h"
#include <fstream>
#include "json.hpp"

PlayerManager::PlayerManager()
{
	//プレイヤーデータの初期化
	std::ifstream file("assets\\data\\playerData.json");
	assert(!file.fail());
	nlohmann::json json;
	file >> json;
	mPlayerData.maxHp = json["maxhp"].get<int>();
	mPlayerData.hp = json["hp"].get<int>();
	mPlayerData.power = json["power"].get<int>();
	mPlayerData.defence = json["defense"].get<int>();
	mPlayerData.moveSpeed = json["moveSpeed"].get<float>();
	mPlayerData.rotSpeed = json["rotSpeed"].get<float>();
	mPlayerData.flushDuration = json["flashDuration"].get<float>();
	mPlayerData.inventory = std::move(json["inventory"].get<std::vector<std::string>>());
	mPlayerData.weaponInventory = std::move(json["weapons"].get<std::vector<std::string>>());
	mPlayerData.armerInventory = std::move(json["armers"].get<std::vector<std::string>>());
	mPlayerData.explorerInventory = std::move(json["explorer"].get<std::vector<std::string>>());
	mPlayerData.equippedWeaponIndex = json["equippedWeaponIndex"].get<int>();
	mPlayerData.equippedArmerIndex = json["equippedArmerIndex"].get<int>();
	mPlayerData.actionLimit = json["actionLimit"].get<int>();
}

const PlayerData& PlayerManager::getPlayerData()
{
	return mPlayerData;
}

const std::string& PlayerManager::getInventoryItem(int index)
{
	int inventorySize = mPlayerData.inventory.size();
	if (inventorySize == 0) return "NONE";

	if (index < 0) index = 0;
	else if (index >= inventorySize) index = inventorySize - 1;
	return mPlayerData.inventory[index];
}

void PlayerManager::setHP(int hp)
{
	mPlayerData.hp = hp;
}

void PlayerManager::setMaxHP(int maxhp) {
	mPlayerData.maxHp = maxhp;
}
 
void PlayerManager::setPower(int power) {
	mPlayerData.power = power;
}

void PlayerManager::setDefence(int defence) {
	mPlayerData.defence = defence;
}

void PlayerManager::equipWeapon(int index)
{
	//配列の範囲内におさまっているか確認
	if (index < 0 || index >= mPlayerData.weaponInventory.size()) return;
	mPlayerData.equippedWeaponIndex = index;
}

void PlayerManager::equipArmer(int index)
{
	//配列の範囲内におさまっているか確認
	if (index < 0 || index >= mPlayerData.armerInventory.size()) return;
	mPlayerData.equippedArmerIndex = index;
}

void PlayerManager::addInventory(std::string id) {
	mPlayerData.inventory.emplace_back(id);
}

void PlayerManager::addWeapon(std::string id)
{
	mPlayerData.weaponInventory.emplace_back(id);
}

void PlayerManager::addArmer(std::string id)
{
	mPlayerData.armerInventory.emplace_back(id);
}

void PlayerManager::addExplorer(std::string id) {
	mPlayerData.explorerInventory.emplace_back(id);
}

void PlayerManager::removeInventory(std::string id) {
	auto iter = std::find(mPlayerData.inventory.begin(), mPlayerData.inventory.end(), id);
	if (iter != mPlayerData.inventory.end()) {
		int index = std::distance(mPlayerData.inventory.begin(), iter);
		std::iter_swap(iter, mPlayerData.inventory.end() - 1);
		mPlayerData.inventory.pop_back();
	}
}

void PlayerManager::removeInventory(int index)
{
	if (index < 0 || index >= mPlayerData.inventory.size()) return;

	//順序を守って削除
	mPlayerData.inventory.erase(mPlayerData.inventory.begin() + index);
}

void PlayerManager::removeWeapon(std::string id) {
	auto iter = std::find(mPlayerData.weaponInventory.begin(), mPlayerData.weaponInventory.end(), id);
	if (iter != mPlayerData.weaponInventory.end()) {
		int index = std::distance(mPlayerData.weaponInventory.begin(), iter);
		std::iter_swap(iter, mPlayerData.weaponInventory.end() - 1);
		mPlayerData.weaponInventory.pop_back();
	}
}

void PlayerManager::removeArmer(std::string id) {
	auto iter = std::find(mPlayerData.armerInventory.begin(), mPlayerData.armerInventory.end(), id);
	if (iter != mPlayerData.armerInventory.end()) {
		int index = std::distance(mPlayerData.armerInventory.begin(), iter);
		//装備中の防具が配列の最後にある場合
		if (mPlayerData.equippedArmerIndex == mPlayerData.armerInventory.size() - 1) mPlayerData.equippedArmerIndex = index;
		
		std::iter_swap(iter, mPlayerData.armerInventory.end() - 1);
		mPlayerData.armerInventory.pop_back();
	}
}

void PlayerManager::removeExplorer(std::string id) {
	auto iter = std::find(mPlayerData.explorerInventory.begin(), mPlayerData.explorerInventory.end(), id);
	if (iter != mPlayerData.explorerInventory.end()) {
		int index = std::distance(mPlayerData.explorerInventory.begin(), iter);
		//装備中の武器が配列の最後にある場合
		if (mPlayerData.equippedWeaponIndex == mPlayerData.weaponInventory.size() - 1) mPlayerData.equippedWeaponIndex = index;
		std::iter_swap(iter, mPlayerData.explorerInventory.end() - 1);
		mPlayerData.explorerInventory.pop_back();
	}
}
