#include "CheckBagManager.h"
#include "PlayerDataHelper.h"
#include "fmt/format.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "ll/api/service/PlayerInfo.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "nlohmann/json.hpp"
#include "player-bag-manager/Config.h"
#include "player-bag-manager/command/PlayerBagManagerCommand.h"
#include <memory>


namespace player_bag_manager::check_bag_manager {

bool CheckBagManager::mIsFree = true;

void CheckBagManager::initUuidNameMap() {
    auto allUuid =
        player_bag_manager::player_data_helper::getAllUuid(!player_bag_manager::config::getConfig().msaIdOnly);
    mUuidNameMap.clear();
    for (auto& uuid : allUuid) {
        auto isFakePlayer = player_bag_manager::player_data_helper::isFakePlayer_ddf8196(uuid);
        mUuidNameMap.emplace(uuid, std::pair{uuid.asString(), isFakePlayer});
    }

    auto playerEntries = ll::service::PlayerInfo::getInstance().entries();
    for (auto iter = playerEntries.begin(); iter != playerEntries.end(); iter++) {
        auto uuidIter = mUuidNameMap.find(iter->uuid);
        if (uuidIter == mUuidNameMap.end()) continue;
        uuidIter->second.first = iter->name;
    }
}

CheckBagManager::CheckBagManager() { initUuidNameMap(); };

CheckBagManager& CheckBagManager::getManager() {
    static CheckBagManager manager;
    return manager;
}

bool CheckBagManager::isCheckingBag(Player* player) {
    auto uuid = player->getUuid();
    return mCheckBagLogMap.find(uuid) != mCheckBagLogMap.end();
}

mce::UUID CheckBagManager::tryGetTargetUuid(Player* player) {
    auto log = tryGetLog(player->getUuid());
    if (log) return log->mTarget;
    return mce::UUID::fromString("");
}

std::string CheckBagManager::getNameOrUuid(mce::UUID const& uuid) {
    auto suuid = uuid.asString();
    auto iter  = mUuidNameMap.find(suuid);
    if (iter != mUuidNameMap.end()) return iter->second.first;
    return suuid;
}

std::string CheckBagManager::getBackupPath(Player* player) {
    auto realName = player->getRealName();
    auto path     = std::filesystem::path(player_bag_manager::config::getConfig().backupDirectory);
    path.append(realName + "." + getSuffix(player_bag_manager::config::getConfig().backupDataType));
    return path.string();
}
std::string CheckBagManager::getExportPath(mce::UUID const& uuid, NbtDataType type) {
    auto        fileName = getNameOrUuid(uuid);
    std::string suffix   = getSuffix(type);
    auto        path     = std::filesystem::path(player_bag_manager::config::getConfig().exportDirectory);
    path.append(fileName + "." + suffix);
    return path.string();
}

std::string CheckBagManager::getSuffix(NbtDataType type) {
    switch (type) {
    case NbtDataType::Snbt:
        return "snbt";
    case NbtDataType::Binary:
        return "nbt";
    case NbtDataType::Json:
        return "json";
    default:
        return "";
    }
}

NbtDataType CheckBagManager::fromSuffix(std::string const& suffix) {
    if (suffix == "snbt") return NbtDataType::Snbt;
    if (suffix == "nbt") return NbtDataType::Binary;
    if (suffix == "json") return NbtDataType::Json;
    return NbtDataType::Unknown;
}

std::string CheckBagManager::getResultI18(CheckBagManager::Result result) {
    switch (result) {
    case CheckBagManager::Result::Success:
        return "manager.result.success";
    case CheckBagManager::Result::Error:
        return "manager.result.error";
    case CheckBagManager::Result::Request:
        return "manager.result.player_online";
    case CheckBagManager::Result::NotStart:
        return "manager.result.not_start";
    case CheckBagManager::Result::BackupError:
        return "manager.result.backup_failed";
    case CheckBagManager::Result::BackupNotFound:
        return "manager.result.backup_not_found";
    case CheckBagManager::Result::TargetNotExist:
        return "manager.result.target_not_exists";
    case CheckBagManager::Result::InfoDataNotFound:
        return "manager.result.info_data_not_found";
    case CheckBagManager::Result::DataTypeNotSupported:
        return "manager.result.data_type_not_supported";
    default:
        return "manager.result.unknown_error";
    }
}


void CheckBagManager::beforePlayerLeave(Player* player) {
    if (isCheckingBag(player)) {
        stopCheckBag(player);
    }
    if (player->isSimulatedPlayer()) {
        auto iter = mCheckBagLogMap.find(player->getUuid());
        if (iter != mCheckBagLogMap.end()) {
            stopCheckBag(ll::service::getLevel()->getPlayer(iter->second.mTarget));
        }
        mCheckBagLogMap.erase(iter);
        command::UpdatePlayerLstSoftEnum(getPlayerList());
    }
}

void CheckBagManager::afterPlayerLeave(Player* player) {
    if (mRemoveRequsets.empty()) return;
    auto uuid     = player->getUuid();
    auto uuidIter = mRemoveRequsets.find(uuid);
    if (uuidIter == mRemoveRequsets.end()) return;
    std::string resultMsg;
    if (player_bag_manager::player_data_helper::removeData(uuid)) {
        resultMsg = fmt::format("Successfully removed Player {} data", player->getRealName());
    } else {
        resultMsg = fmt::format("An error has occurred while removing Player {} data", player->getRealName());
    }

    auto logPlayer = ll::service::getLevel()->getPlayer(uuidIter->second);
    mRemoveRequsets.erase(uuidIter);
    updateIsFree();
    if (logPlayer) logPlayer->sendMessage(resultMsg);
    // logger.info(format, player->getRealName());
}

void CheckBagManager::afterPlayerJoin(Player* player) {
    auto uuid = player->getUuid();
    if (mUuidNameMap.find(uuid) == mUuidNameMap.end()) {
        auto isFakePlayer =
            player_bag_manager::player_data_helper::isFakePlayer_ddf8196(uuid) || player->isSimulatedPlayer();
        mUuidNameMap.emplace(uuid, std::pair{player->getRealName(), isFakePlayer});
        command::UpdatePlayerLstSoftEnum(getPlayerList());
    }
    auto backupTag = getBackupBag(player);
    if (!backupTag) return;
    mIsFree = false;
    mCheckBagLogMap.emplace(uuid, CheckBagLog(uuid, std::move(backupTag)));
    player->sendMessage("plugin.warn.backup_found");
}

mce::UUID CheckBagManager::getUUIDFromName(std::string const& name) {
    return ll::service::PlayerInfo::getInstance().fromName(name)->uuid;
    ;
}

std::vector<std::string> CheckBagManager::getPlayerList() {
    std::vector<std::string> playerList;
    playerList.resize(mUuidNameMap.size());
    size_t index  = 0;
    size_t rindex = mUuidNameMap.size() - 1;
    for (auto& [uuid, value] : mUuidNameMap) {
        auto& name = value.first;
        // if (name == uuid)
        if (name.size() == 36) playerList[rindex--] = uuid.asString();
        else playerList[index++] = name;
    }
    std::sort(playerList.begin(), playerList.begin() + index);
    std::sort(playerList.begin() + index, playerList.end());
    return playerList;
}

std::vector<std::string> CheckBagManager::getPlayerList(PlayerCategory category) {
    if (category == PlayerCategory::All) return getPlayerList();
    std::vector<std::string> playerList;
    for (auto& [uuid, value] : mUuidNameMap) {
        auto& name = value.first;
        // TestFuncTime(mUuidNameMap.isFakePlayer, suuid); // <=1
        if (value.second) {
            if (category != PlayerCategory::FakePlayer) continue;
            playerList.push_back(name);
            continue;
        } else {
            if (name.size() == 36) {
                if (category == PlayerCategory::Unnamed) playerList.push_back(uuid.asString());
            } else if (category == PlayerCategory::Normal) playerList.push_back(name);
            continue;
        }
    }
    // TestFuncTime(std::sort, playerList.begin(), playerList.end(), nameSortFunc); //  100 - 200
    // std::sort(playerList.begin(), playerList.end(), nameSortFunc);
    std::sort(playerList.begin(), playerList.end(), [](std::string const& _Left, std::string const& _Right) {
        if (_Left.size() == 36) {
            if (_Right.size() == 36) return player_bag_manager::player_data_helper::nameSortFunc(_Left, _Right);
            else return false;
        } else {
            if (_Right.size() == 36) return true;
            else return player_bag_manager::player_data_helper::nameSortFunc(_Left, _Right);
        }
    });
    return playerList;
}

std::vector<std::pair<PlayerCategory, std::vector<std::string>>> CheckBagManager::getClassifiedPlayerList() {
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> playerList;
    std::vector<std::string>                                         normalList;
    std::vector<std::string>                                         fakePlayerList;
    std::vector<std::string>                                         unnamedFakePlayerList;
    std::vector<std::string>                                         unnamedList;

    using namespace player_bag_manager::player_data_helper;

    for (auto& [uuid, value] : mUuidNameMap) {
        auto& name = value.first;
        if (value.second) {
            if (name.size() == 36) fakePlayerList.push_back(uuid.asString());
            else unnamedFakePlayerList.push_back(name);
            continue;
        } else {
            if (name.size() == 36) unnamedList.push_back(uuid.asString());
            else normalList.push_back(name);
            continue;
        }
    }

    std::sort(normalList.begin(), normalList.end(), nameSortFunc);
    std::sort(fakePlayerList.begin(), fakePlayerList.end(), nameSortFunc);
    std::sort(unnamedFakePlayerList.begin(), unnamedFakePlayerList.end());
    std::sort(unnamedList.begin(), unnamedList.end());
    fakePlayerList.insert(fakePlayerList.end(), unnamedFakePlayerList.begin(), unnamedFakePlayerList.end());
    if (normalList.size()) playerList.emplace_back(PlayerCategory::Normal, std::move(normalList));
    if (fakePlayerList.size()) playerList.emplace_back(PlayerCategory::FakePlayer, std::move(fakePlayerList));
    if (unnamedList.size()) playerList.emplace_back(PlayerCategory::Unnamed, std::move(unnamedList));
    return playerList;
}

std::unique_ptr<CompoundTag> CheckBagManager::getBackupBag(Player* player) {
    if (auto log = tryGetLog(player->getUuid())) {
        std::unique_ptr<CompoundTag> tag = {};
        log->mBackup.swap(tag);
        mCheckBagLogMap.erase(player->getUuid());
        updateIsFree();
        return tag;
    } else {
        auto path = getBackupPath(player);
        auto bin  = ll::utils::file_utils::readFile(
            path,
            player_bag_manager::config::getConfig().backupDataType == NbtDataType::Binary
        );
        if (bin.has_value())
            return player_bag_manager::player_data_helper::deserializeNbt(
                std::move(bin.value()),
                player_bag_manager::config::getConfig().backupDataType
            );
        return {};
    }
}

CheckBagManager::Result CheckBagManager::removePlayerData(Player* player) {
    auto uuid = player->getUuid();
    mIsFree   = false;
    mRemoveRequsets.emplace(uuid, player->getOrCreateUniqueID().rawID);
    return Result::Request;
}

CheckBagManager::Result CheckBagManager::removePlayerData(mce::UUID const& uuid) {
    if (!uuid) return Result::Error;
    if (auto player = ll::service::getLevel()->getPlayer(uuid)) {
        mIsFree = false;
        mRemoveRequsets.emplace(uuid, player->getOrCreateUniqueID().rawID);
        return Result::Success;
    }
    if (player_bag_manager::player_data_helper::removeData(uuid)) {
        mUuidNameMap.erase(uuid);
        updateIsFree();
        return Result::Success;
    }
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::setCheckBagLog(Player* player, mce::UUID const& target, CompoundTag& tag) {
    if (auto log = tryGetLog(player->getUuid())) {
        log->mTarget = target;
        return Result::Success;
    }
    auto&& data = player_bag_manager::player_data_helper::serializeNbt(
        tag.clone(),
        player_bag_manager::config::getConfig().backupDataType
    );
    if (ll::utils::file_utils::writeFile(getBackupPath(player), data, true)) {
        mIsFree = false;
        mCheckBagLogMap.emplace(player->getUuid(), CheckBagLog(target, tag.clone()));
        return Result::Success;
    }
    return Result::BackupError;
}

CheckBagManager::Result CheckBagManager::overwriteBagData(Player* player, CheckBagLog const& log) {
    auto        targetPlayer = ll::service::getLevel()->getPlayer(log.mTarget);
    CompoundTag data;
    player->save(data);
    if (targetPlayer) {
        if (player_bag_manager::player_data_helper::setPlayerBag(targetPlayer, data)) return Result::Success;
        return Result::Error;
    }
    if (player_bag_manager::player_data_helper::writePlayerBag(log.mTarget, data)) return Result::Success;
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::restoreBagData(Player* player) {
    if (player_bag_manager::config::getConfig().packetMode) {
        player->refreshInventory();
        return Result::Success;
    } else {
        auto backupPath = getBackupPath(player);
        auto backupTag  = getBackupBag(player);
        if (!backupTag) return Result::BackupNotFound;
        player_bag_manager::player_data_helper::setPlayerBag(player, *backupTag);
        player->refreshInventory();
        std::filesystem::remove(backupPath);
        return Result::Success;
    }
}

CheckBagManager::Result
CheckBagManager::setBagData(Player* player, mce::UUID const& uuid, std::unique_ptr<CompoundTag> targetTag) {
    if (player_bag_manager::config::getConfig().packetMode) {
        // sendBagData();
        return Result::Error;
    } else {
        CompoundTag playerTag;
        player->save(playerTag);
        auto result = setCheckBagLog(player, uuid, playerTag);

        if (result == Result::Success) {
            auto                  res = player_bag_manager::player_data_helper::changeBagTag(playerTag, *targetTag);
            DefaultDataLoadHelper dataloadhelper;
            // TODO 下面这句未验证是否有效
            player->readAdditionalSaveData(playerTag, dataloadhelper);
            ;
            player->refreshInventory();
            if (res) return Result::Success;
            return Result::Error;
        };
        return result;
    }
}

CheckBagManager::Result CheckBagManager::stopCheckBag(Player* player) {
    if (!tryGetLog(player->getUuid())) return Result::NotStart;
    auto rtn = restoreBagData(player);
    updateIsFree();
    return rtn;
}

CheckBagManager::Result CheckBagManager::startCheckBag(Player* player, Player* target) {
    auto uuid = target->getUuid();

    auto playerTag = std::make_unique<CompoundTag>();
    player->save(*playerTag);
    return setBagData(player, uuid, std::move(playerTag));
}

CheckBagManager::Result CheckBagManager::startCheckBag(Player* player, mce::UUID const& uuid) {
    if (auto target = ll::service::getLevel()->getPlayer(uuid)) return startCheckBag(player, target);
    auto targetTag = player_bag_manager::player_data_helper::getPlayerTag(uuid);
    if (!targetTag || targetTag->isEmpty()) return Result::TargetNotExist;
    return setBagData(player, uuid, std::move(targetTag));
}

CheckBagManager::Result CheckBagManager::checkNext(Player* player) {
    // 相对于所有玩家
    auto log = tryGetLog(player->getUuid());
    if (!log) return Result::NotStart;
    auto target     = log->mTarget;
    auto targetName = getNameOrUuid(target);
    auto list       = getPlayerList(log->mCategory);
    auto iter       = std::find(list.begin(), list.end(), targetName);
    // ASSERT(iter != list.end());
    ++iter;
    if (iter == list.end()) iter = list.begin();
    if (*iter == player->getRealName()) {
        ++iter;
        if (iter == list.end()) iter = list.begin();
    }
    player->sendMessage("operation.check_next.hint");
    return startCheckBag(player, mce::UUID(*iter) ? mce::UUID(*iter) : getUUIDFromName(*iter));
}

CheckBagManager::Result CheckBagManager::checkPrevious(Player* player) {
    auto log = tryGetLog(player->getUuid());
    if (!log) return Result::NotStart;
    auto target     = log->mTarget;
    auto targetName = getNameOrUuid(target);
    auto list       = getPlayerList(log->mCategory);
    auto iter       = std::find(list.rbegin(), list.rend(), targetName);
    // ASSERT(iter != list.rend());
    ++iter;
    if (iter == list.rend()) iter = list.rbegin();
    if (*iter == player->getRealName()) {
        ++iter;
        if (iter == list.rend()) iter = list.rbegin();
    }
    player->sendMessage("operation.check_previous.hint");
    return startCheckBag(player, mce::UUID(*iter) ? mce::UUID(*iter) : getUUIDFromName(*iter));
}

CheckBagManager::Result CheckBagManager::overwriteData(Player* player) {
    auto log = tryGetLog(player->getUuid());
    if (!log) return Result::NotStart;
    auto rtn = overwriteBagData(player, *log);
    restoreBagData(player);
    updateIsFree();
    return rtn;
}

CheckBagManager::Result CheckBagManager::exportData(mce::UUID const& uuid, NbtDataType type = NbtDataType::Snbt) {
    if (!uuid) return Result::Error;
    std::string data;
    if (!ll::service::getLevel()->getPlayer(uuid) && type == NbtDataType::Binary) {
        data = player_bag_manager::player_data_helper::getPlayerData(uuid);
    } else {
        std::unique_ptr<CompoundTag> tag = player_bag_manager::player_data_helper::getExpectedPlayerTag(uuid);
        if (!tag) return Result::Error;
        data = player_bag_manager::player_data_helper::serializeNbt(std::move(tag), type);
    }

    if (data.empty()) return Result::Error;

    auto           playerName = ll::service::PlayerInfo::getInstance().fromUuid(uuid)->name;
    auto           idsTag     = player_bag_manager::player_data_helper::getPlayerIdsTag(uuid);
    nlohmann::json playerInfo;
    playerInfo["name"] = playerName;
    playerInfo["uuid"] = uuid.asString();
    playerInfo["data"] = idsTag->toNetworkNbt();
    // if (idsTag) {
    //     for (auto& [type, idTag] : *idsTag) {
    //         playerInfo[type] = const_cast<CompoundTagVariant&>(idTag).asStringTag()->value();
    //     }
    // }

    auto infoStr = playerInfo.dump(4);

    auto        dataPath = getExportPath(uuid, type);
    std::string infoPath = dataPath + ".json";
    if (ll::utils::file_utils::writeFile(dataPath, data, true)
        && ll::utils::file_utils::writeFile(infoPath, infoStr, false))
        return Result::Success;
    return Result::Error;
}

CheckBagManager::Result
CheckBagManager::exportData(std::string const& nameOrUuid, NbtDataType type = NbtDataType::Snbt) {
    if (auto uuid = mce::UUID(nameOrUuid) ? mce::UUID(nameOrUuid) : getUUIDFromName(nameOrUuid))
        return exportData(uuid, type);
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::importData(mce::UUID const& uuid, std::string filePath, bool isBagOnly) {
    if (!uuid || filePath.empty()) return Result::Error;
    auto suffix = filePath.substr(filePath.find_last_of('.') + 1);
    auto newTag = player_bag_manager::player_data_helper::readTagFile(filePath, fromSuffix(suffix));
    if (!newTag) return Result::Error;
    if (isBagOnly) {
        if (auto player = ll::service::getLevel()->getPlayer(uuid)) {
            player_bag_manager::player_data_helper::setPlayerBag(player, *newTag);
        } else {
            auto oldTag = player_bag_manager::player_data_helper::getExpectedPlayerTag(uuid);
            if (!oldTag) return Result::Error;
            if (player_bag_manager::player_data_helper::writePlayerBag(uuid, *newTag)) return Result::Success;
            return Result::Error;
        }
    } else {
        if (ll::service::getLevel()->getPlayer(uuid)) return Result::Error;
        if (player_bag_manager::player_data_helper::writePlayerData(uuid, *newTag)) return Result::Success;
    }
    return Result::Error;
}

CheckBagManager::Result
CheckBagManager::importData(std::string const& nameOrUuid, std::string filePath, bool isBagOnly) {
    return Result::Error;
    if (auto uuid = mce::UUID(nameOrUuid) ? mce::UUID(nameOrUuid) : getUUIDFromName(nameOrUuid))
        return importData(uuid, filePath, isBagOnly);
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::importNewData(std::string filePath) {
    if (!std::filesystem::exists(filePath + ".json")) {
        // logger.error("Failed to get player info file，file {} not exist", filePath + ".json");
        return Result::InfoDataNotFound;
    }
    auto infoData = ll::utils::file_utils::readFile(filePath + ".json");

    auto        playerIds = nlohmann::json::parse(infoData.value_or("{}"));
    CompoundTag idsTag{playerIds};
    std::string name  = idsTag["name"];
    std::string suuid = idsTag["uuid"];
    // for (auto& [key, val] : playerIds.items()) {
    //     std::string id = val.get<std::string>();
    //     if (key == "name") {
    //         name = id;
    //         continue;
    //     }
    //     if (key == "uuid") {
    //         suuid = id;
    //         continue;
    //     }
    // }
    if (suuid.empty()) {
        suuid = idsTag["MsaId"];
        if (suuid.empty() && !player_bag_manager::config::getConfig().msaIdOnly) suuid = idsTag["SelfSignedId"];
    }
    if (suuid.empty()) return Result::Error;

    auto suffix = filePath.substr(filePath.find_last_of('.') + 1);
    auto type   = fromSuffix(suffix);
    if (type != NbtDataType::Binary && type != NbtDataType::Snbt) return Result::DataTypeNotSupported;
    auto data = ll::utils::file_utils::readFile(filePath, NbtDataType::Binary == type);
    auto tag  = player_bag_manager::player_data_helper::deserializeNbt(data.value_or(""));
    if (!tag || idsTag.isEmpty()) {
        return Result::Error;
    }
    auto idsTag_ptr = std::make_unique<CompoundTag>(idsTag);
    if (player_bag_manager::player_data_helper::writeNewPlayerData(std::move(idsTag_ptr), std::move(tag))) {
        bool isFakePlayer = player_bag_manager::player_data_helper::isFakePlayer_ddf8196(suuid);
        mUuidNameMap.emplace(suuid, std::pair{(name.empty() ? suuid : name), isFakePlayer});
        return Result::Success;
    }
    return Result::Error;
}

size_t CheckBagManager::exportAllData(NbtDataType type) {
    size_t count = 0;
    for (auto& suuid : getPlayerList()) {
        auto result = exportData(suuid, type);
        if (result == CheckBagManager::Result::Success) count++;
        else {
            // logger.warn(tr("operation.export.failed"), suuid, CheckBagManager::getResultString(result));
        }
    }
    return count;
}

LL_TYPE_INSTANCE_HOOK(
    PlayerLeftEventHook,
    HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::_onPlayerLeft,
    void,
    ServerPlayer* player,
    bool          skipMessage
) {
    if (CheckBagManager::mIsFree) return origin(player, skipMessage);

    auto& manager = CheckBagManager::getManager();
    // 保存玩家数据前
    manager.beforePlayerLeave(player);
    origin(player, skipMessage);
    // 玩家数据保存后
    manager.afterPlayerLeave(player);
}

} // namespace player_bag_manager::check_bag_manager