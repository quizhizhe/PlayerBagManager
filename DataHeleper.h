#pragma once
#include "pch.h"
#include "SymHelp.h"
#include "JsonHelper.h"
#include <MC/LevelStorage.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Player.hpp>
#include <MC/PlayerDataSystem.hpp>
#include <MC/EndTag.hpp>
#include <MC/ByteTag.hpp>
#include <MC/ShortTag.hpp>
#include <MC/IntTag.hpp>
#include <MC/Int64Tag.hpp>
#include <MC/FloatTag.hpp>
#include <MC/DoubleTag.hpp>
#include <MC/ByteArrayTag.hpp>
#include <MC/StringTag.hpp>
#include <MC/ListTag.hpp>
#include <third-party/Nlohmann/json.hpp>
#include <Global.h>
#include <LLAPI.h>


bool writeBagData(std::string playerTag, std::string &bagdata);
std::string loadBagData(std::string playerTag);
bool recoveBagData(Player* player);
std::unique_ptr<class CompoundTag> loadPlayer(Player* player);
std::unique_ptr<class CompoundTag> loadPlayer(const string& uuid);
void saveBag(Player* player);//No complete
vector<string> getPlayerButton();
void copyBagData(Player* masterP, std::unique_ptr<CompoundTag> target);
//void copyBagData(std::string uuid1, std::string uuid2);
std::string getServerId(Player* player);