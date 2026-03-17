#include "InnMenu.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "json.hpp"

InnMenu::InnMenu(Game* game, float zDepth) : Menu(game, "InnMenu", zDepth)
{
	mMaxIndex = 2;
}

void InnMenu::selectedAct()
{
	switch (mSelectedIndex) {
	case 0:
		stay();
		break;
	case 1:
		save();
		break;
	}
}

void InnMenu::stay()
{
	mGame->getPlayerManager()->setHP(mGame->getPlayerManager()->getPlayerData().maxHp);
}

void InnMenu::save()
{
	//リソースデータの保存
	{
		nlohmann::json itemJson;
		std::ifstream itemFile("assets/data/itemData.json");
		itemFile >> itemJson;
		itemFile.close();

		for (auto& resource : itemJson["Resource"]) {
			resource["num"] = mGame->getItemManager()->getResourceNum(resource["id"]);
		}

		//一時ファイルへの書き出し
		try {
			std::ofstream os("assets/data/itemData.json.tmp");
			if (!os) throw std::runtime_error("ファイルが開けません");

			os << itemJson.dump(4);
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		//書き込みが成功したら、一時ファイルを正式な名前にリネームする
		try {
			// すでに正式なファイルが存在する場合は上書きされる
			std::filesystem::rename("assets/data/itemData.json.tmp", "assets/data/itemData.json");
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}

	//プレイヤーデータの保存
	{
		nlohmann::json playerJson;
		std::ifstream playerFile("assets/data/playerData.json");
		playerFile >> playerJson;
		playerFile.close();
		const PlayerData& playerData = mGame->getPlayerManager()->getPlayerData();
		playerJson["hp"] = playerData.hp;
		playerJson["inventory"] = playerData.inventory;
		playerJson["weapons"] = playerData.weaponInventory;
		playerJson["armers"] = playerData.armerInventory;
		playerJson["explorer"] = playerData.explorerInventory;
		playerJson["equippedWeaponIndex"] = playerData.equippedWeaponIndex;
		playerJson["equippedArmerIndex"] = playerData.equippedArmerIndex;

		//一時ファイルへの書き出し
		try {
			std::ofstream os("assets/data/playerData.json.tmp");
			if (!os) throw std::runtime_error("ファイルが開けません");

			os << playerJson.dump(4);
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		//書き込みが成功したら、一時ファイルを正式な名前にリネームする
		try {
			// すでに正式なファイルが存在する場合は上書きされる
			std::filesystem::rename("assets/data/playerData.json.tmp", "assets/data/playerData.json");
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << e.what() << std::endl;
		}
	}

}

