#pragma once
#include <vector>
#include <string>

struct PlayerData {
	int maxHp = 0;
	int hp = 0;
	int power = 0;
	int defence = 0;
	int actionLimit = 0;
	float moveSpeed = 0.0f;
	float rotSpeed = 0.0f;
	float flushDuration = 0.0f;
	std::vector<std::string> inventory;
	std::vector<std::string> weaponInventory;
	std::vector<std::string> armerInventory;
	std::vector<std::string> explorerInventory;
	int equippedWeaponIndex = 0;
	int equippedArmerIndex = 0;
};

class PlayerManager
{
public:
	PlayerManager();

	const PlayerData& getPlayerData();
	const std::string& getInventoryItem(int index);

	void setHP(int hp);
	void setMaxHP(int maxhp);
	void setPower(int poewr);
	void setDefence(int defence);
	void equipWeapon(int index);
	void equipArmer(int index);

	void addInventory(std::string id);
	void addWeapon(std::string id);
	void addArmer(std::string id);
	void addExplorer(std::string id);

	void removeInventory(std::string id);
	void removeInventory(int index);
	void removeWeapon(std::string id);
	void removeArmer(std::string id);
	void removeExplorer(std::string id);

private:
	PlayerData mPlayerData;
};

