#pragma once

#include "Config.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/platform/UUID.h"


class Player;

namespace player_bag_manager::player_data_helper {

bool nameSortFunc(std::string const& left, std::string const& right);

void forEachUuid(bool allowSelfSignedId, std::function<void(std::string_view const& uuid)> callback);

std::vector<mce::UUID> getAllUuid(bool allowSelfSignedId);

std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid);

bool removeData(mce::UUID const& uuid);

std::string getServerId(mce::UUID const& uuid);

std::string getPlayerData(mce::UUID const& uuid);

std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID const& uuid);

std::unique_ptr<CompoundTag> getExpectedPlayerTag(mce::UUID const& uuid);

bool writePlayerData(mce::UUID const& uuid, CompoundTag& data);

bool changeBagTag(CompoundTag& dst, CompoundTag& src);

bool setPlayerBag(Player* player, CompoundTag& data);

bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data);

std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type = NbtDataType::Binary);

std::unique_ptr<CompoundTag> deserializeNbt(std::string const& data, NbtDataType type = NbtDataType::Binary);

bool isFakePlayer_ddf8196(mce::UUID const& uuid);

std::unique_ptr<CompoundTag> readTagFile(std::string const& path, NbtDataType type = NbtDataType::Binary);

bool writeNewPlayerData(std::unique_ptr<CompoundTag> idsTag, std::unique_ptr<CompoundTag> dataTag);

} // namespace player_bag_manager::player_data_helper
