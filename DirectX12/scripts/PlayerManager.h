#pragma once
#include <vector>
#include <string>

class Game;

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
	int storageSize = 1;
};

class PlayerManager
{
public:
	PlayerManager(Game& game);

	const PlayerData& getPlayerData();
	const std::string& getInventoryItem(int index);
	const std::vector<std::string>& getInventory();

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

	void applyToolParamater();

	void loadPlayerData();
private:
	PlayerData mPlayerData;
	PlayerData mDefaultPlayerData; //プレイヤーデータの初期値を保持する変数
	Game& mGame;
};

