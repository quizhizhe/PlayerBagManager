#include "PlayerDataHelper.h"
#include "PlayerBagManager.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/io/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "mc/dataloadhelper/DataLoadHelper.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/Minecraft.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/storage/LevelStorage.h"
#include "mc/world/level/storage/db_helpers/Category.h"
#include "player-bag-manager/Config.h"
#include <memory>
#include <string_view>


namespace player_bag_manager::player_data_helper {
std::string const PLAYER_KEY_SERVER_ID      = "ServerId";
std::string const PLAYER_KEY_MSA_ID         = "MsaId";
std::string const PLAYER_KEY_SELF_SIGNED_ID = "SelfSignedId";

DBHelpers::Category const playerCategory = DBHelpers::Category::Player;


bool nameSortFunc(std::string const& left, std::string const& right) {
    size_t maxSize = std::max(left.size(), right.size());

    for (size_t i = 0; i < maxSize; i++) {
        auto c1 = left[i];
        auto c2 = right[i];
        if (c1 == c2) continue;
        auto tmp = tolower(c1) - tolower(c2);
        if (tmp == 0) return c1 > c2;
        return tmp < 0;
    }
    return right.size() - left.size();
}

void forEachUuid(bool includeSelfSignedId, std::function<void(std::string_view const& uuid)> callback) {
    static size_t count;
    count = 0;

    ll::service::getLevel()->getLevelStorage().forEachKeyWithPrefix(
        "player_",
        playerCategory,
        [&callback, includeSelfSignedId](std::string_view key_left, std::string_view data) {
            if (key_left.size() == 36) {
                auto         tag   = CompoundTag::fromBinaryNbt(data, data.size()).value();
                std::string& msaId = tag[PLAYER_KEY_MSA_ID];
                if (!msaId.empty()) {
                    if (msaId == key_left) {
                        count++;
                        callback(msaId);
                    }
                    return;
                }
                if (!includeSelfSignedId) return;
                std::string& selfSignedId = tag[PLAYER_KEY_SELF_SIGNED_ID];
                if (!selfSignedId.empty()) {
                    if (selfSignedId == key_left) {
                        count++;
                        callback(selfSignedId);
                    }
                    return;
                }
            }
        }
    );
}

std::vector<mce::UUID> getAllUuid(bool includeSelfSignedId) {
    std::vector<mce::UUID> uuids;

    forEachUuid(includeSelfSignedId, [&uuids](std::string_view uuid) { uuids.emplace_back(mce::UUID{uuid}); });
    return uuids;
}
std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid) {
    auto playerKey = "player_" + uuid.asString();
    if (ll::service::getLevel()->getLevelStorage().hasKey(playerKey, playerCategory)) {
        return ll::service::getLevel()->getLevelStorage().getCompoundTag(playerKey, playerCategory);
    }
    return {};
}
bool removeData(mce::UUID const& uuid) {
    try {
        auto& levelStorage = ll::service::getLevel()->getLevelStorage();
        auto  serverId     = getServerId(uuid);
        if (serverId.empty()) return false;
        if (!levelStorage.hasKey(serverId, playerCategory)) {
            getLogger().warn("Failed to find key {0} when deleting player({1})'s data", serverId, uuid.asString());
            return false;
        }
        auto res = levelStorage.deleteData(serverId, playerCategory);
        return true;
    } catch (const std::exception&) {
        getLogger().error("Error in PlayerDataHelper::removeData");
    }
    return false;
}
std::string getServerId(mce::UUID const& uuid) {
    auto tag = getPlayerIdsTag(uuid);
    return tag->mTags[PLAYER_KEY_SERVER_ID];
}
std::string getPlayerData(mce::UUID const& uuid) {
    auto serverId = getServerId(uuid);
    if (serverId.empty()) return {};
    if (!ll::service::getLevel()->getLevelStorage().hasKey(serverId, playerCategory)) return {};
    std::string data = "";
    if (ll::service::getLevel()->getLevelStorage().loadData(serverId, data, playerCategory)) return data;
    else return "";
}
std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID const& uuid) {
    auto serverId = getServerId(uuid);
    if (serverId.empty()) return {};
    if (!ll::service::getLevel()->getLevelStorage().hasKey(serverId, playerCategory)) return {};
    return ll::service::getLevel()->getLevelStorage().getCompoundTag(serverId, playerCategory);
}
std::unique_ptr<CompoundTag> getExpectedPlayerTag(mce::UUID const& uuid) {
    if (auto player = ll::service::getLevel()->getPlayer(uuid)) {
        auto playerNbt = std::make_unique<CompoundTag>();
        player->save(*playerNbt);
        return std::move(playerNbt);
    }
    return getPlayerTag(uuid);
}
bool writePlayerData(mce::UUID const& uuid, CompoundTag& data) {
    try {
        auto serverId = getServerId(uuid);
        if (serverId.empty()) return false;
        ll::service::getLevel()->getLevelStorage().saveData(serverId, data.toBinaryNbt(), playerCategory);
        return true;
    } catch (const std::exception&) {
        getLogger().error("Error in PlayerDataHelper::writePlayerData");
    }
    return false;
}
bool changeBagTag(CompoundTag& dst, CompoundTag& src) {
    try {
        if (!&dst || !&src) return false;
        auto res = true;
        // TODO 以下的判断会不会有问题？需要实际测试
        res = res && (dst["Armor"] = src["Armor"]);
        res = res && (dst["EnderChestInventory"] = src["EnderChestInventory"]);
        res = res && (dst["Inventory"] = src["Inventory"]);
        res = res && (dst["Mainhand"] = src["Mainhand"]);
        res = res && (dst["Offhand"] = src["Offhand"]);
        return res;
    } catch (const std::exception&) {
        getLogger().error("Error in PlayerDataHelper::changeBagTag");
        return false;
    }
}
bool setPlayerBag(Player* player, CompoundTag& data) {
    auto                  res = true;
    CompoundTag           playerTag;
    DefaultDataLoadHelper dataloadhelper;
    player->save(playerTag);
    res = res && changeBagTag(playerTag, data);
    // TODO 下面这句未验证是否有效
    player->readAdditionalSaveData(data, dataloadhelper);
    player->refreshInventory();
    return res;
}
bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data) {
    try {
        auto res       = true;
        auto playerTag = getPlayerTag(uuid);
        if (!playerTag) res = false;
        res = res && changeBagTag(*playerTag, data);
        return res && writePlayerData(uuid, *playerTag);
    } catch (const std::exception&) {
        getLogger().error("Error in PlayerDataHelper::writePlayerBag");
    }
    return false;
}

std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type) {
    if (!tag) return "";
    switch (type) {
    case NbtDataType::Snbt:
        return tag->toSnbt(
            player_bag_manager::config::getConfig().formattedSNBT ? SnbtFormat::Minimize : SnbtFormat::ArrayLineFeed
        );
    case NbtDataType::Binary:
        return tag->toBinaryNbt();
    case NbtDataType::Json:
        return tag->toString();
    case NbtDataType::Unknown:
    default:
        return "";
        break;
    }
}
std::unique_ptr<CompoundTag> deserializeNbt(std::string const& data, NbtDataType type) {
    if (data.empty()) return {};
    switch (type) {
    case NbtDataType::Snbt: {
        auto tag = CompoundTag::fromSnbt(data);
        if (tag) return std::make_unique<CompoundTag>(std::move(*tag));
        else return {};
    }
    case NbtDataType::Binary: {
        auto tag = CompoundTag::fromBinaryNbt(data);
        if (tag) return std::make_unique<CompoundTag>(std::move(*tag));
        else return {};
    }
    case NbtDataType::Json:
    case NbtDataType::Unknown:
    default:
        return {};
    }
}

bool isFakePlayer_ddf8196(mce::UUID const& uuid) {
    auto tag = getPlayerIdsTag(uuid);
    if (!tag) return false;
    return tag->mTags[PLAYER_KEY_MSA_ID] == tag->mTags[PLAYER_KEY_SELF_SIGNED_ID];
}

std::unique_ptr<CompoundTag> readTagFile(std::string const& path, NbtDataType type) {
    if (type != NbtDataType::Binary && type != NbtDataType::Snbt) return {};
    auto data = ll::utils::file_utils::readFile(path, type == NbtDataType::Binary);
    if (!data.has_value()) return {};
    return deserializeNbt(data.value(), type);
}
bool writeNewPlayerData(std::unique_ptr<CompoundTag> idsTag, std::unique_ptr<CompoundTag> dataTag) {
    try {
        // TODO 需要测试是否可以先修改，再序列化，然后saveData
        // auto idsData = serializeNbt(idsTag->clone(), NbtDataType::Binary);
        // auto data = serializeNbt(std::move(dataTag), NbtDataType::Binary);
        // std::string serverId = "";
        // for (auto& [type, idTag] : *idsTag) {
        //     std::string id = const_cast<CompoundTagVariant&>(idTag).asStringTag()->value();

        //     switch (do_hash(type.c_str()))
        //     {
        //     case do_hash("MsaId"):
        //     case do_hash("PlatformOfflineId"):
        //     case do_hash("PlatformOnlineId"):
        //     case do_hash("SelfSignedId"):
        //         ll::service::getLevel()->getLevelStorage().saveData("player_" + id, std::string(idsData),
        //         playerCategory); break;
        //     case do_hash("ServerId"):
        //         serverId = id;
        //         break;
        //     default:
        //         break;
        //     }
        // }
        // ll::service::getLevel()->getLevelStorage().saveData(serverId, std::move(data), playerCategory);
        return true;
    } catch (const std::exception&) {
        getLogger().error("Error in PlayerDataHelper::writeNewPlayerData");
        return false;
    }
    return false;
}
} // namespace player_bag_manager::player_data_helper

// TODO 到时候移动到别的文件夹
DefaultDataLoadHelper::DefaultDataLoadHelper()=default;
DataLoadHelper::DataLoadHelper()=default;