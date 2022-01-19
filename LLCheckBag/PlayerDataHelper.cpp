#include "pch.h"
#include "PlayerDataHelper.h"
#include <MC/DBStorage.hpp>
#include <MC/StringTag.hpp>
#include <MC/VanillaBlocks.hpp>
namespace PlayerDataHelper {
    DBHelpers::Category const playerCategory = (DBHelpers::Category)7;
    std::string const PLAYER_KEY_SERVER_ID = "ServerId";
    std::string const PLAYER_KEY_MSA_ID = "MsaId";
    std::string const PLAYER_KEY_SELF_SIGNED_ID = "SelfSignedId";

    void forEachUuid(bool includeSelfSignedId, std::function<void(std::string_view const& uuid)> callback)
    {
        static size_t count;
        static size_t serverCount;
        serverCount = 0;
        count = 0;
        Global<DBStorage>->forEachKeyWithPrefix("player_", playerCategory,
            [&callback, includeSelfSignedId](gsl::cstring_span<-1> key_left, gsl::cstring_span<-1> data) {
                if (key_left.size() == 36) {
                    auto tag = CompoundTag::fromBinaryNBT((void*)data.data(), data.size());
                    auto& msaId = tag->getString(PLAYER_KEY_MSA_ID);
                    //logger.warn("{}", key_left.data());
                    //logger.warn(tag->toJson(4));
                    if (!msaId.empty()) {
                        if (msaId == key_left) {
                            count++;
                            callback(msaId);
                        }
                        return;
                    }
                    if (!includeSelfSignedId)
                        return;
                    auto& selfSignedId = tag->getString(PLAYER_KEY_SELF_SIGNED_ID);
                    //if (msaId.empty() && selfSignedId.empty())
                    //    __debugbreak();
                    if (!selfSignedId.empty()) {
                        if (selfSignedId == key_left) {
                            count++;
                            callback(selfSignedId);
                        }
                        return;
                    }
                }
                else if(key_left.size() == 36+7) {
                    serverCount++;
                }
            });
        if (serverCount != count) {
            logger.warn("获取玩家id数据时发现异常数据");
            //logger.error("ServerId1");
            //for (auto& id : allServerId) {
            //    if (std::find(allServerId2.begin(), allServerId2.end(), id) == allServerId2.end())
            //        logger.warn(id);
            //}
            //logger.error("ServerId2");
            //for (auto& id : allServerId2) {
            //    if (allServerId.find(id) == allServerId.end())
            //        logger.warn(id);
            //}
            //__debugbreak();
        }
    }

    std::vector<string> getAllUuid(bool includeSelfSignedId)
    {
        std::vector<std::string> uuids;
        //std::vector<std::string> serverIds;
        forEachUuid(includeSelfSignedId, [&uuids](std::string_view uuid) {
            //auto u = mce::UUID::fromString(std::string(uuid));
            //auto sid = getServerId(u);
            //if (std::find(serverIds.begin(), serverIds.end(), sid) != serverIds.end())
            //    __debugbreak();
            //serverIds.push_back(sid);
            uuids.push_back(std::string(uuid));
            //logger.info("{}", uuid);
            });
        return uuids;
    }
    std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid) {
        auto& dbStorage = *Global<DBStorage>;
        auto playerKey = "player_" + uuid.asString();
        if (dbStorage.hasKey(playerKey, playerCategory)) {
            return dbStorage.getCompoundTag(playerKey, playerCategory);
        }
        return {};
    }
    bool removeData(mce::UUID const& uuid)
    {
        auto& dbStorage = *Global<DBStorage>;
        auto tag = getPlayerIdsTag(uuid);
        if (!tag) {
            //logger.error("key \"player_{}\" not found in storage", uuid.asString());
            return false;
        }
        for (auto& [key, idTagVariant] : *tag) {
            auto idTag = const_cast<CompoundTagVariant&>(idTagVariant).asStringTag();
            std::string id = idTag->value();
            id = key == PLAYER_KEY_SERVER_ID ? id : "player_" + id;
            if (!dbStorage.hasKey(id, playerCategory)) {
                logger.warn("Failed to find key {} when deleting player({})'s data", id, uuid.asString());
                continue;
            }
            auto res = dbStorage.deleteData(id, playerCategory);
            //res->addOnComplete([id](Bedrock::Threading::IAsyncResult<void> const& result) {
            //    if (result.getStatus() == 1)
            //        dbLogger.warn("Remove {} Success", id);
            //    else
            //    {
            //        auto code = result.getError();
            //        dbLogger.error("Remove {} Failed, cause: {}", id, code.message());
            //    }
            //    });
        }
        return true;
    }
    std::string getServerId(mce::UUID const& uuid) {
        auto tag = getPlayerIdsTag(uuid);
        if (!tag)
            return "";
        return tag->getString(PLAYER_KEY_SERVER_ID);
    }
    std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID const& uuid)
    {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return {};
        if (!Global<DBStorage>->hasKey(serverId, playerCategory))
            return {};
        return Global<DBStorage>->getCompoundTag(serverId, playerCategory);
    }
    bool writePlayerData(mce::UUID const& uuid, CompoundTag& data) {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return false;
        Global<DBStorage>->saveData(serverId, data.toBinaryNBT(), playerCategory);
        return true;
    }
    bool changeBagTag(CompoundTag& dst, CompoundTag& src) {
        if (!&dst || !&src)
            return false;
        auto res = true;
        res = res && dst.put("Armor", src.get("Armor")->copy());
        res = res && dst.put("EnderChestInventory", src.get("EnderChestInventory")->copy());
        res = res && dst.put("Inventory", src.get("Inventory")->copy());
        res = res && dst.put("Mainhand", src.get("Mainhand")->copy());
        res = res && dst.put("Offhand", src.get("Offhand")->copy());
        return res;
    }
    bool setPlayerBag(Player* player, CompoundTag& data) {
        auto res = true;
        auto playerTag = player->getNbt();
        res = res && changeBagTag(*playerTag, data);
        res = res && player->setNbt(playerTag.get());
        res = res && player->refreshInventory();
        return res;
    }
    bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data) {
        auto res = true;
        auto playerTag = getPlayerTag(uuid);
        if (!playerTag)
            res = false;
        res = res && changeBagTag(*playerTag, data);
        return res && writePlayerData(uuid, *playerTag);
    }

    std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type) {
        if (!tag)
            return "";
        switch (type)
        {
        case NbtDataType::Snbt:
            return tag->toSNBT();
        case NbtDataType::Binary:
            return tag->toBinaryNBT();
        case NbtDataType::Json:
            return tag->toJson(4);
        case NbtDataType::Unknown:
        default:
            return "";
            break;
        }
    }
    std::unique_ptr<CompoundTag> deserializeNbt(std::string const& data, NbtDataType type) {
        switch (type)
        {
        case NbtDataType::Snbt:
            return CompoundTag::fromSNBT(data);
        case NbtDataType::Binary:
            return CompoundTag::fromBinaryNBT((void*)data.c_str(), data.size());
        case NbtDataType::Json:
        case NbtDataType::Unknown:
        default:
            return {};
        }
    }

    bool isFakePlayer_ddf8196(mce::UUID const& uuid) {
        auto tag = getPlayerIdsTag(uuid);
        if (!tag)
            return false;
        return tag->getString(PLAYER_KEY_MSA_ID) == tag->getString(PLAYER_KEY_SELF_SIGNED_ID);
    }

    bool isFakePlayer_ddf8196(std::string const& suuid) {
        auto uuid = mce::UUID::fromString(suuid);
        return isFakePlayer_ddf8196(uuid);
    }
}
