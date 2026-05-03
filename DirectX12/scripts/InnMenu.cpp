#include "InnMenu.h"
#include "TownScene.h"
#include "Game.h"
#include "PlayerManager.h"
#include "ItemManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "json.hpp"
#include "AudioManager.h"
#include "Graphic.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "input.h"

InnMenu::InnMenu(TownScene& scene, float zDepth) : Menu(scene, "InnMenu", zDepth)
{
	mMaxIndex = 2;
	isFading = false;
	mSleepVoice = nullptr;

	addComponentLabel("descriptor", "TextComponent");
	applyComponentLabel();
}

void InnMenu::selectedAct()
{
	switch (mSelectedIndex) {
	case 0:
		stay();
		break;
	case 1:
		auto saveWindow = std::make_unique<ConfirmWindow>(mScene, *this);
		mScene.addActor(std::move(saveWindow));
		mScene.getGame().getAudioManager().playSE("UI_WINDOW_OPEN");
		break;
	}
}

void InnMenu::updateMenu()
{
	if (mSleepVoice) {

		XAUDIO2_VOICE_STATE state;
		mSleepVoice->GetState(&state);
		if (state.BuffersQueued == 0) {
			//HPを全回復
			mScene.getGame().getPlayerManager().setHP(mScene.getGame().getPlayerManager().getPlayerData().maxHp);
			//UIに反映
			mScene.updateStatusWindow();
			mScene.getGame().getGraphic().startFadeIn(1.0f);
			mScene.getGame().getAudioManager().playBGM("BGM_TOWN");
			mSleepVoice = nullptr;
		}
	}
}

void InnMenu::inputMenu()
{
	if (mArrow) {
		if (isKeyJustPressed(VK_UP) || isKeyJustPressed('W')) {
			if (mSelectedIndex <= 0) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex--;
			mArrow->movePosition(XMFLOAT2(0.0f, -mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}

		if (isKeyJustPressed(VK_DOWN) || isKeyJustPressed('S')) {
			if (mSelectedIndex >= mMaxIndex - 1) {
				mScene.getGame().getAudioManager().playSE("UI_CANCEL");
				return;
			}
			mSelectedIndex++;
			mArrow->movePosition(XMFLOAT2(0.0f, mArrowMoveLength));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
			//説明文の更新
			updateDescriptor();
		}
	}
}

void InnMenu::updateActor()
{
	if (mDescriptor) {
		//メニューがアクティブでないとき
		if (mScene.getCurrentMenu() != this) {
			mDescriptor->setPosZ(200.0f);
		}
		else
			mDescriptor->setPosZ(mDescriptorDefaultZ);
	}
}

void InnMenu::applyComponentLabel()
{
	mDescriptor = static_cast<TextComponent*>(mComponentLabels["descriptor"].pComponent);
	if (mDescriptor) {
		mDescriptorDefaultZ = mDescriptor->getPosZ();
		updateDescriptor();
	}
}

void InnMenu::stay()
{
	//SE
	mScene.getGame().getAudioManager().finishAllSounds();
	mSleepVoice = mScene.getGame().getAudioManager().playSE("SLEEP");
	mScene.getGame().getGraphic().startFadeOut(2.0f);
}

void InnMenu::save()
{
	//アイテムデータの更新
	{
		nlohmann::json itemJson;
		std::ifstream itemFile("assets/data/itemData.json");
		itemFile >> itemJson;
		itemFile.close();

		//リソースの所持数を更新
		for (auto& resource : itemJson["Resource"]) {
			resource["num"] = mScene.getGame().getItemManager().getResourceNum(resource["id"]);
		}
		//武器の所有を更新
		for (auto& weapon : itemJson["Weapon"]) {
			weapon["inPossession"] = mScene.getGame().getItemManager().getWeaponData(weapon["id"]).inPossession;
		}
		//防具の所有を更新
		for (auto& armer : itemJson["Armer"]) {
			armer["inPossession"] = mScene.getGame().getItemManager().getArmerData(armer["id"]).inPossession;
		}
		//探索道具の所有を更新
		for (auto& tool : itemJson["Explorer"]) {
			tool["inPossession"] = mScene.getGame().getItemManager().getExplorerData(tool["id"]).inPossession;
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
		const PlayerData& playerData = mScene.getGame().getPlayerManager().getPlayerData();
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

void InnMenu::updateDescriptor()
{
	if (!mDescriptor) return;

	std::wstring text;
	switch (mSelectedIndex) {
	case 0:
		text = L"HPを全回復します。\n";
		break;
	case 1:
		text = L"セーブします。\n";
		break;
	}

	mDescriptor->setText(text);
}

ConfirmWindow::ConfirmWindow(TownScene& scene, InnMenu& menu)
	:Menu(scene, "ConfirmMenu", 10.0f)
{
	mMaxIndex = 2;
	mArrowMoveLength = 181.0f;
}

void ConfirmWindow::inputMenu()
{
	if (isKeyJustPressed(VK_RIGHT) || isKeyJustPressed('D')) {
		if (mSelectedIndex >= mMaxIndex - 1) return;
		mSelectedIndex++;
		mArrow->movePosition(XMFLOAT2(mArrowMoveLength, 0.0f));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
	}
	else if (isKeyJustPressed(VK_LEFT) || isKeyJustPressed('A')) {
		if (mSelectedIndex <= 0) return;
		mSelectedIndex--;
		mArrow->movePosition(XMFLOAT2(-mArrowMoveLength, 0.0f));
			mScene.getGame().getAudioManager().playSE("UI_MOVE1");
	}
}

void ConfirmWindow::selectedAct()
{
	switch (mSelectedIndex) {
	case 0:
		save();
		mScene.popMenu();
		mScene.getGame().getAudioManager().playSE("UI_ENTER");
		break;
	case 1:
		mScene.popMenu();
		mScene.getGame().getAudioManager().playSE("UI_WINDOW_CLOSE");
		break;
	}
}

void ConfirmWindow::save()
{
	//アイテムデータの更新
	{
		nlohmann::json itemJson;
		std::ifstream itemFile("assets/data/itemData.json");
		itemFile >> itemJson;
		itemFile.close();

		//リソースの所持数を更新
		for (auto& resource : itemJson["Resource"]) {
			resource["num"] = mScene.getGame().getItemManager().getResourceNum(resource["id"]);
		}
		//武器の所有を更新
		for (auto& weapon : itemJson["Weapon"]) {
			weapon["inPossession"] = mScene.getGame().getItemManager().getWeaponData(weapon["id"]).inPossession;
		}
		//防具の所有を更新
		for (auto& armer : itemJson["Armer"]) {
			armer["inPossession"] = mScene.getGame().getItemManager().getArmerData(armer["id"]).inPossession;
		}
		//探索道具の所有を更新
		for (auto& tool : itemJson["Explorer"]) {
			tool["inPossession"] = mScene.getGame().getItemManager().getExplorerData(tool["id"]).inPossession;
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
		const PlayerData& playerData = mScene.getGame().getPlayerManager().getPlayerData();
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
