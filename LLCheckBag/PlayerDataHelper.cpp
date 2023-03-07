#include "pch.h"
#include "PlayerDataHelper.h"
#include <llapi/mc/DBStorage.hpp>
#include <llapi/mc/StringTag.hpp>

namespace PlayerDataHelper {
    DBHelpers::Category const playerCategory = (DBHelpers::Category)7;
    std::string const PLAYER_KEY_SERVER_ID = "ServerId";
    std::string const PLAYER_KEY_MSA_ID = "MsaId";
    std::string const PLAYER_KEY_SELF_SIGNED_ID = "SelfSignedId";

    void forEachUuid(bool includeSelfSignedId, std::function<void(std::string_view const& uuid)> callback)
    {
        static size_t count;
        count = 0;
#ifdef DEBUG
        static size_t serverCount;
        serverCount = 0;
#endif // DEBUG
        Global<DBStorage>->forEachKeyWithPrefix("player_", /*playerCategory,*/
            [&callback, includeSelfSignedId](gsl::cstring_span<-1> key_left, gsl::cstring_span<-1> data) {
                if (key_left.size() == 36) {
                    auto tag = CompoundTag::fromBinaryNBT((void*)data.data(), data.size());
                    auto& msaId = tag->getString(PLAYER_KEY_MSA_ID);
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
                    if (!selfSignedId.empty()) {
                        if (selfSignedId == key_left) {
                            count++;
                            callback(selfSignedId);
                        }
                        return;
                    }
                }
#ifdef DEBUG
                else if (key_left.size() == 36 + 7) {
                    serverCount++;
                }
            });
        if (serverCount != count) {
            logger.warn("获取玩家id数据时发现异常数据，玩家数据数量:{} 和UUID数量:{} 不同", serverCount, count);
        }
#else
    });
#endif // DEBUG
    }

    std::vector<string> getAllUuid(bool includeSelfSignedId)
    {
        std::vector<std::string> uuids;

        forEachUuid(includeSelfSignedId, [&uuids](std::string_view uuid) {
            uuids.push_back(std::string(uuid));
            });
#ifdef DEBUG
        std::unordered_map<std::string, std::string> serverIds;
        for (auto& uuid : uuids) {
            auto serverId = getServerId(mce::UUID::fromString(uuid));
            if (serverIds.find(serverId) != serverIds.end()) {
                logger.error("发现超过一个UUID拥有相同的ServerId");
                logger.error("UUID 1  : {}", uuid);
                logger.error("UUID 2  : {}", serverIds[serverId]);
                logger.error("ServerId: {}", serverId);
                continue;
            }
            if (!Global<DBStorage>->hasKey(serverId, playerCategory)) {
                logger.error("未找到ServerId \"{}\" 对应的数据", serverId);
            }
            serverIds.emplace(serverId, uuid);
        }
#endif // DEBUG
        return uuids;
    }
    std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid) {
        auto& dbStorage = *Global<DBStorage>;
        auto playerKey = "player_" + uuid.asString();
        if (dbStorage.hasKey(playerKey/*, playerCategory*/)) {
            return dbStorage.getCompoundTag(playerKey/*, playerCategory*/);
        }
        return {};
    }
    bool removeData(mce::UUID const& uuid)
    {
        try
        {
            auto& dbStorage = *Global<DBStorage>;
            auto serverId = getServerId(uuid);
            if (serverId.empty())
                return false;
            if (!dbStorage.hasKey(serverId/*, playerCategory*/)) {
                logger.warn("Failed to find key {} when deleting player({})'s data", serverId, uuid.asString());
                return false;
            }
            auto res = dbStorage.deleteData(serverId/*, playerCategory*/);
            return true;

            //auto tag = getPlayerIdsTag(uuid);
            //if (!tag) {
            //    //logger.error("key \"player_{}\" not found in storage", uuid.asString());
            //    return false;
            //}
            // 
            // Remove all player keys
            //for (auto& [key, idTagVariant] : *tag) {
            //    auto idTag = const_cast<CompoundTagVariant&>(idTagVariant).asStringTag();
            //    std::string id = idTag->value();
            //    id = key == PLAYER_KEY_SERVER_ID ? id : "player_" + id;
            //    if (!dbStorage.hasKey(id, playerCategory)) {
            //        logger.warn("Failed to find key {} when deleting player({})'s data", id, uuid.asString());
            //        continue;
            //    }
            //    auto res = dbStorage.deleteData(id, playerCategory);
            //    //res->addOnComplete([id](Bedrock::Threading::IAsyncResult<void> const& result) {
            //    //    if (result.getStatus() == 1)
            //    //        dbLogger.warn("Remove {} Success", id);
            //    //    else
            //    //    {
            //    //        auto code = result.getError();
            //    //        dbLogger.error("Remove {} Failed, cause: {}", id, code.message());
            //    //    }
            //    //    });
            //}
            return true;
        }
        catch (const std::exception&)
        {
            logger.error("Error in PlayerDataHelper::removeData");
        }
        return false;
    }
    std::string getServerId(mce::UUID const& uuid) {
        auto tag = getPlayerIdsTag(uuid);
        if (!tag)
            return "";
        return tag->getString(PLAYER_KEY_SERVER_ID);
    }
    std::string getPlayerData(mce::UUID const& uuid)
    {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return {};
        if (!Global<DBStorage>->hasKey(serverId/*, playerCategory*/))
            return {};
        std::string data = "";
        if (Global<DBStorage>->loadData(serverId, data/*, playerCategory*/))
            return data;
        else
            return "";
    }
    std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID const& uuid)
    {
        auto serverId = getServerId(uuid);
        if (serverId.empty())
            return {};
        if (!Global<DBStorage>->hasKey(serverId/*, playerCategory*/))
            return {};
        return Global<DBStorage>->getCompoundTag(serverId/*, playerCategory*/);
    }
    std::unique_ptr<CompoundTag> getExpectedPlayerTag(mce::UUID const& uuid)
    {
        if (auto player = getPlayer(uuid)) {
            return player->getNbt();
        }
        return getPlayerTag(uuid);
    }
    bool writePlayerData(mce::UUID const& uuid, CompoundTag& data) {
        try
        {
            auto serverId = getServerId(uuid);
            if (serverId.empty())
                return false;
            Global<DBStorage>->saveData(serverId, data.toBinaryNBT()/*, playerCategory*/);
            return true;
        }
        catch (const std::exception&)
        {
            logger.error("Error in PlayerDataHelper::writePlayerData");
        }
        return false;
    }
    bool changeBagTag(CompoundTag& dst, CompoundTag& src) {
        try
        {
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
        catch (const std::exception&)
        {
            logger.error("Error in PlayerDataHelper::changeBagTag");
            return false;
        }
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
        try
        {
            auto res = true;
            auto playerTag = getPlayerTag(uuid);
            if (!playerTag)
                res = false;
            res = res && changeBagTag(*playerTag, data);
            return res && writePlayerData(uuid, *playerTag);
        }
        catch (const std::exception&)
        {
            logger.error("Error in PlayerDataHelper::writePlayerBag");
        }
        return false;
    }

    std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type) {
        if (!tag)
            return "";
        switch (type)
        {
        case NbtDataType::Snbt:
            return tag->toSNBT(4, 
                Config::FormattedSNBT ? SnbtFormat::PartialNewLine : SnbtFormat::Minimize);
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
        if (data.empty())
            return {};
        switch (type)
        {
        case NbtDataType::Snbt:
            return CompoundTag::fromSNBT(data);
        case NbtDataType::Binary:
            return CompoundTag::fromBinaryNBT((void*)data.c_str(), data.size(), true);
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

    std::unique_ptr<CompoundTag> readTagFile(std::string const& path, NbtDataType type)
    {
        if (type != NbtDataType::Binary && type != NbtDataType::Snbt)
            return {};
        auto data = ReadAllFile(path, type == NbtDataType::Binary);
        if (!data.has_value())
            return {};
        return deserializeNbt(data.value(), type);
    }
    bool writeNewPlayerData(std::unique_ptr<CompoundTag> idsTag, std::unique_ptr<CompoundTag> dataTag) {
        try
        {
            auto idsData = serializeNbt(idsTag->clone(), NbtDataType::Binary);
            auto data = serializeNbt(std::move(dataTag), NbtDataType::Binary);
            std::string serverId = "";
            for (auto& [type, idTag] : *idsTag) {
                std::string id = const_cast<CompoundTagVariant&>(idTag).asStringTag()->value();

                switch (do_hash(type.c_str()))
                {
                case do_hash("MsaId"):
                case do_hash("PlatformOfflineId"):
                case do_hash("PlatformOnlineId"):
                case do_hash("SelfSignedId"):
                    Global<DBStorage>->saveData("player_" + id, std::string(idsData)/*, playerCategory*/);
                    break;
                case do_hash("ServerId"):
                    serverId = id;
                    break;
                default:
                    break;
                }
            }
            Global<DBStorage>->saveData(serverId, std::move(data)/*, playerCategory*/);
            return true;
        }
        catch (const std::exception&)
        {
            logger.error("Error in PlayerDataHelper::writeNewPlayerData");
            return false;
        }
        return false;
    }
}
