#pragma once
#include <MC/Level.hpp>

namespace PlayerDataHelper {
    void forEachUuid(bool allowSelfSignedId, std::function<void(std::string_view const& uuid)> callback);
    std::vector<string> getAllUuid(bool allowSelfSignedId);
    std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid);
    bool removeData(mce::UUID const& uuid);
    std::string getServerId(mce::UUID const& uuid);
    std::unique_ptr<CompoundTag> getPlayerData(mce::UUID const& uuid);
    bool writePlayerData(mce::UUID const& uuid, CompoundTag& data);
    bool changeBagTag(CompoundTag& dst, CompoundTag& src);
    bool setPlayerBag(Player* player, CompoundTag& data);
    bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data);
    std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type = NbtDataType::Binary);
    std::unique_ptr<CompoundTag> deserializeNbt(std::string const& data, NbtDataType type = NbtDataType::Binary);
    bool isFakePlayer_ddf8196(mce::UUID const& uuid);
}
