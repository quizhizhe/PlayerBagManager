#include "pch.h"
#include "DataHeleper.h"
#include <filesystem>
#include <cstdio>
#include <boost/format.hpp>
#include <fstream>
#include <algorithm>

boost::format BAG_DATA_PATH = boost::format("./plugins/LLCheckBag/%s.json");
using namespace std;
using namespace nlohmann;

bool writeBagData(std::string filePath, std::string& bagdata) {
	ofstream of(filePath);
	if (of) {
		of << bagdata;
		of.flush();
		of.close();
		return true;
	}
	else {
		cerr << "[LLCheckBag] save Bag data error!!!" << endl;
		return false;
	}
	return false;
}

std::string loadBagData(std::string filePath) {
	ifstream fs;
	fs.open(filePath, ios::in);
	if (fs) {
		ostringstream json;
		char ch;
		while (json&&fs.get(ch)) {
			json.put(ch);
		}
		fs.close();
		return json.str();
	}
	else {
		cerr << "[LLCheckBag] load Bag data error!!!" << endl;
		return "";
	}
	return "";
}
//»Ö¸´±³°ü
bool recoveBagData(Player* player) {

	BAG_DATA_PATH% player->getRealName();
	auto filePath = BAG_DATA_PATH.str();
	std::cout << filePath << std::endl;
	std::string playerN = player->getRealName();
	std::unique_ptr<CompoundTag> plData = player->getNbt();
	std::string snbt = loadBagData(filePath);
	auto backData = CompoundTag::fromSNBT(snbt);

	auto inventory = (ListTag*)backData->getListTag("Inventory");
	auto offhand = (ListTag*)backData->getListTag("Offhand");
	auto armor = (ListTag*)backData ->getListTag("Armor");
	auto enderChestInventory = (ListTag*)backData->getListTag("EnderChestInventory");
	if (inventory) {
		plData->put("Inventory", inventory->copy());
	};
	if (offhand) {
		plData->put("Offhand", offhand->copy());
	};
	if (armor) {
		plData->put("Armor", armor->copy());
	};
	if (enderChestInventory) {
		plData->put("EnderChestInventory", enderChestInventory->copy());
	};
	if (player->setNbt(&*plData)) {
		player->refreshInventory();
		remove(BAG_DATA_PATH.str().c_str());
		player->sendTextPacket("You Inventory is recovery");
		return true;
	}
	player->sendTextPacket("You Inventory isn't recovery!!Try again");
	return false;
}

std::unique_ptr<class CompoundTag> loadPlayer(Player* player) {
	return player->getNbt();
};
std::unique_ptr<class CompoundTag> loadPlayer(const std::string& uuid) {
	LevelStorage* levelStorage = ::Global<LevelStorage>;
	return PlayerDataSystem::loadPlayerDataFromTag(*levelStorage, uuid);
};

//backup bag
void saveBag(Player* player) {
	if (!player) return;
	auto plData = player->getNbt();
	std::unique_ptr<CompoundTag> saveNBT= CompoundTag::create();
	auto inventory = (ListTag*)plData->getListTag("Inventory");
	auto offhand = (ListTag*)plData->getListTag("Offhand");
	auto armor = (ListTag*)plData->getListTag("Armor");
	auto enderChestInventory = (ListTag*)plData->getListTag("EnderChestInventory");
	if (inventory) {
		saveNBT->put("Inventory",inventory->copy());
	};
	if (offhand) {
		saveNBT->put("Offhand",offhand->copy());
	};
	if (armor) {
		saveNBT->put("Armor",armor->copy());
	};
	if (enderChestInventory) {
		saveNBT->put("EnderChestInventory",enderChestInventory->copy());
	};
	auto snbt = saveNBT->toSNBT();
	player->refreshInventory();
	BAG_DATA_PATH% player->getRealName();
	writeBagData(BAG_DATA_PATH.str(), snbt);
	player->sendTextPacket("You Inventory is backup!");
};

vector<string> getPlayerButton() {
	vector<string> plIdList = loadAllPlayerIDs();
	for (int i = 0; i < plIdList.size(); i++) {
		std::string id = findUuid(plIdList[i]);
		if (!id.empty()) {
			plIdList[i] = getNameByUuid(id);
		}
	}
	sort(plIdList.begin(), plIdList.end());//size is small to large
	return plIdList;
};
//¸´ÖÆ±³°ü
void copyBagData(Player* masterP, std::unique_ptr<CompoundTag> target) {
	auto master = masterP->getNbt();
	auto inventoryT = (ListTag*)target->getListTag("Inventory");
	auto armorT = (ListTag*)target->getListTag("Armor");
	auto offhandT = (ListTag*)target->getListTag("Offhand");
	auto enderChestInventoryT = (ListTag*)target->getListTag("EnderChestInventory");
	if (inventoryT) {
		master->put("Inventory", inventoryT->copy());
	};
	if (offhandT) {
		master->put("Offhand", offhandT->copy());
	};
	if (armorT) {
		master->put("Armor", armorT->copy());
	};
	if (enderChestInventoryT) {
		master->put("EnderChestInventory", enderChestInventoryT->copy());
	};
	masterP->setNbt(&*master);
	masterP->refreshInventory();
};
/*
void copyBagData(std::string uuid1, std::string uuid2) {
	LevelStorage* levelStorage = ::Global<LevelStorage>;
	auto master = PlayerDataSystem::loadPlayerDataFromTag(*levelStorage,uuid1);
	auto target = PlayerDataSystem::loadPlayerDataFromTag(*levelStorage, uuid2);
	auto inventoryT = (ListTag*)target->getListTag("Inventory");
	auto armorT = (ListTag*)target->getListTag("Armor");
	auto offhandT = (ListTag*)target->getListTag("Offhand");
	auto enderChestInventoryT = (ListTag*)target->getListTag("EnderChestInventory");
	if (inventoryT) {
		master->put("Inventory", inventoryT->copy());
	};
	if (offhandT) {
		master->put("Offhand", offhandT->copy());
	};
	if (armorT) {
		master->put("Armor", armorT->copy());
	};
	if (enderChestInventoryT) {
		master->put("EnderChestInventory", enderChestInventoryT->copy());
	};
	masterP->setNbt(&*master);
	masterP->refreshInventory();
};*/

std::string getServerId(Player* player) {
	LevelStorage* levelStorage = ::Global<LevelStorage>;
	return PlayerDataSystem::serverKey(*levelStorage, *player);
}