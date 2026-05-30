#include "PlayerManager.h"
#include "Game.h"
#include "ItemManager.h"
#include <fstream>
#include "json.hpp"
#include "PlayerAttackComponent.h"

static std::string empty = "";

PlayerManager::PlayerManager(Game& game) : mGame(game)
{
}

const PlayerData& PlayerManager::getPlayerData()
{
	return mPlayerData;
}

const std::string& PlayerManager::getInventoryItem(int index)
{
	int inventorySize = mPlayerData.inventory.size();
	if (inventorySize == 0 || index >= inventorySize || index < 0) return empty;

	return mPlayerData.inventory[index];
}

const std::vector<std::string>& PlayerManager::getInventory()
{
	return mPlayerData.inventory;
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
	mPlayerData.power = mDefaultPlayerData.power + mGame.getItemManager().getWeaponData(mPlayerData.weaponInventory[index]).power;
}

void PlayerManager::equipArmer(int index)
{
	//配列の範囲内におさまっているか確認
	if (index < 0 || index >= mPlayerData.armerInventory.size()) return;
	mPlayerData.equippedArmerIndex = index;
	mPlayerData.defence = mDefaultPlayerData.defence + mGame.getItemManager().getArmerData(mPlayerData.armerInventory[index]).defence;
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

void PlayerManager::applyEquipmentParamater()
{
	//防具の適用
	int defence = mDefaultPlayerData.defence;
	defence += mGame.getItemManager().getArmerData(mPlayerData.armerInventory[mPlayerData.equippedArmerIndex]).defence;
	mPlayerData.defence = defence;

	//武器の適用
	int power = mDefaultPlayerData.power;
	power += mGame.getItemManager().getWeaponData(mPlayerData.weaponInventory[mPlayerData.equippedWeaponIndex]).power;
	mPlayerData.power = power;

}

void PlayerManager::applyToolParamater()
{
	//ストレージサイズを初期化
	mPlayerData.storageSize = mDefaultPlayerData.storageSize;
	for (auto& toolID : mPlayerData.explorerInventory) {
		auto toolData = mGame.getItemManager().getExplorerData(toolID);
		std::string category = toolData.category;
		if (category == "ACTION_LIMIT") {
			mPlayerData.actionLimit = mDefaultPlayerData.actionLimit + toolData.value;
		}
		else if (category == "STORAGE_SIZE") {
			mPlayerData.storageSize++;
		}
		else if (category == "YIELD") {
			mGame.getItemManager().addResourceYield(toolData.valueID, toolData.value);
		}
		else if (category == "DOUBLE_ATTACK") {
			mPlayerData.attackType = AttackType::DOUBLE;
		}
	}
}

void PlayerManager::loadPlayerData()
{
	//プレイヤーデータの初期化
	std::ifstream file("assets\\data\\playerData.json");
	assert(!file.fail());
	nlohmann::json json;
	file >> json;
	mDefaultPlayerData.maxHp = json["maxhp"].get<int>();
	mDefaultPlayerData.hp = json["hp"].get<int>();
	mDefaultPlayerData.power = json["power"].get<int>();
	mDefaultPlayerData.defence = json["defense"].get<int>();
	mDefaultPlayerData.moveSpeed = json["moveSpeed"].get<float>();
	mDefaultPlayerData.rotSpeed = json["rotSpeed"].get<float>();
	mDefaultPlayerData.flushDuration = json["flashDuration"].get<float>();
	mDefaultPlayerData.inventory = std::move(json["inventory"].get<std::vector<std::string>>());
	mDefaultPlayerData.weaponInventory = std::move(json["weapons"].get<std::vector<std::string>>());
	mDefaultPlayerData.armerInventory = std::move(json["armers"].get<std::vector<std::string>>());
	mDefaultPlayerData.explorerInventory = std::move(json["explorer"].get<std::vector<std::string>>());
	mDefaultPlayerData.equippedWeaponIndex = json["equippedWeaponIndex"].get<int>();
	mDefaultPlayerData.equippedArmerIndex = json["equippedArmerIndex"].get<int>();
	mDefaultPlayerData.actionLimit = json["actionLimit"].get<int>();
	mDefaultPlayerData.attackType = AttackType::SINGLE;	//攻撃タイプの初期値はシングルアタックにする

	mPlayerData = mDefaultPlayerData; //プレイヤーデータを初期値で初期化:
}
