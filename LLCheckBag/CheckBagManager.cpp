#include "pch.h"
#include "CheckBagManager.h"
#include "PlayerDataHelper.h"
#include <PlayerInfoAPI.h>
#include <FormUI.h>
#include <MC/StringTag.hpp>

bool CheckBagManager::mIsFree = true;

void CheckBagManager::initUuidNameMap() {
    auto allUuid = PlayerDataHelper::getAllUuid(!Config::MsaIdOnly);
    mUuidNameMap.clear();
    for (auto& suuid : allUuid) {
        auto isFakePlayer = PlayerDataHelper::isFakePlayer_ddf8196(suuid);
        mUuidNameMap.emplace(suuid, std::pair{ suuid, isFakePlayer });
    }
    PlayerInfo::forEachInfo([this](std::string_view name, std::string_view xuid, std::string_view uuid) ->bool {
        auto suuid = std::string(uuid);
        auto iter = mUuidNameMap.find(suuid);
        if (iter == mUuidNameMap.end())
            return true;
        iter->second.first = name;
        return true;
        });
}

CheckBagManager::CheckBagManager() {
    TestFuncTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestFuncTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestFuncTime(initUuidNameMap);
    TestFuncTime(initUuidNameMap);
    initUuidNameMap();
};

CheckBagManager& CheckBagManager::getManager()
{
    static CheckBagManager manager;
    return manager;
}

std::string CheckBagManager::getSuffix(NbtDataType type)
{
    switch (type)
    {
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

NbtDataType CheckBagManager::fromSuffix(std::string const& suffix)
{
    if (suffix == "snbt")
        return NbtDataType::Snbt;
    if (suffix == "nbt")
        return NbtDataType::Binary;
    if (suffix == "json")
        return NbtDataType::Json;
    return NbtDataType::Unknown;
}

void CheckBagManager::beforePlayerLeave(ServerPlayer* player)
{
    if (isCheckingBag(player)) {
        stopCheckBag(player);
    }
}

void CheckBagManager::afterPlayerLeave(ServerPlayer* player)
{
    if (mRemoveRequsets.empty())
        return;
    auto suuid = player->getUuid();
    auto uuid = mce::UUID::fromString(suuid);
    auto uuidIter = mRemoveRequsets.find(suuid);
    if (uuidIter == mRemoveRequsets.end())
        return;
    auto res = PlayerDataHelper::removeData(uuid);
    auto logPlayer = Level::getPlayer(uuidIter->second);
    mRemoveRequsets.erase(uuidIter);
    updateIsFree();
    auto format = res ? "成功移除玩家 {} 数据" : "移除玩家 {} 数据时发生错误";
    if (logPlayer)
        logPlayer->sendText(fmt::format(format, player->getRealName()));
    logger.info(format, player->getRealName());

}

void CheckBagManager::afterPlayerJoin(ServerPlayer* player) {
    auto suuid = player->getUuid();
    if (mUuidNameMap.find(suuid) != mUuidNameMap.end()) {
        auto isFakePlayer = PlayerDataHelper::isFakePlayer_ddf8196(suuid);
        mUuidNameMap.emplace(suuid, std::pair{ player->getRealName(),isFakePlayer });
    }
    auto backupTag = getBackupBag(player);
    if (!backupTag)
        return;
    mIsFree = false;
    auto uuid = player->getUuid();
    mCheckBagLogMap.emplace(uuid, CheckBagLog(mce::UUID::fromString(uuid), std::move(backupTag)));
    player->sendText("发现有备份文件，建议先使用llcb stop指令恢复背包后在进行查包");
}

mce::UUID CheckBagManager::fromNameOrUuid(std::string const& nameOrUuid) {
    auto uuid = mce::UUID::fromString(nameOrUuid);
    if (!uuid)
        return mce::UUID::fromString(PlayerInfo::getUUID(nameOrUuid));
    return mce::UUID::fromString("");
}

std::vector<std::string> CheckBagManager::getPlayerList() {
    std::vector<std::string> playerList;
    playerList.resize(mUuidNameMap.size());
    size_t index = 0;
    size_t rindex = mUuidNameMap.size() - 1;
    for (auto& [suuid, value] : mUuidNameMap) {
        auto& name = value.first;
        //if (name == uuid)
        if (name.size() == 36)
            playerList[rindex--] = suuid;
        else
            playerList[index++] = name;
    }
    std::sort(playerList.begin(), playerList.begin() + index, nameSortFunc);
    std::sort(playerList.begin() + index, playerList.end());
    return playerList;
}

std::vector<std::string> CheckBagManager::getPlayerList(PlayerCategory category) {
    if (category == PlayerCategory::All)
        return getPlayerList();
    std::vector<std::string> playerList;
    size_t index = 0;
    size_t rindex = mUuidNameMap.size() - 1;
    for (auto& [suuid, value] : mUuidNameMap) {
        auto& name = value.first;
        //TestFuncTime(mUuidNameMap.isFakePlayer, suuid); // <=1
        if (value.second) {
            if (category != PlayerCategory::FakePlayer)
                continue;
            playerList.push_back(name);
            continue;
        }
        else {
            if (name.size() == 36) {
                if (category == PlayerCategory::Unnamed)
                    playerList.push_back(suuid);
            }
            else if (category == PlayerCategory::Normal)
                playerList.push_back(name);
            continue;
        }
    }
    //TestFuncTime(std::sort, playerList.begin(), playerList.end(), nameSortFunc); //  100 - 200
    std::sort(playerList.begin(), playerList.end(), nameSortFunc);
    return playerList;
}

std::vector<std::pair<PlayerCategory, std::vector<std::string>>> CheckBagManager::getClassifiedPlayerList() {
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> playerList;
    std::vector<std::string> normalList;
    std::vector<std::string> fakePlayerList;
    std::vector<std::string> unnamedFakePlayerList;
    std::vector<std::string> unnamedList;
    for (auto& [suuid, value] : mUuidNameMap) {
        auto& name = value.first;
        if (value.second) {
            if (name.size() == 36)
                fakePlayerList.push_back(suuid);
            else
                unnamedFakePlayerList.push_back(name);
            continue;
        }
        else {
            if (name.size() == 36)
                unnamedList.push_back(suuid);
            else
                normalList.push_back(name);
            continue;

        }
    }

    std::sort(normalList.begin(), normalList.end(), nameSortFunc);
    std::sort(fakePlayerList.begin(), fakePlayerList.end(), nameSortFunc);
    std::sort(unnamedFakePlayerList.begin(), unnamedFakePlayerList.end());
    std::sort(unnamedList.begin(), unnamedList.end());
    fakePlayerList.insert(fakePlayerList.end(), unnamedFakePlayerList.begin(), unnamedFakePlayerList.end());
    if (normalList.size())
        playerList.emplace_back(PlayerCategory::Normal, std::move(normalList));
    if (fakePlayerList.size())
        playerList.emplace_back(PlayerCategory::FakePlayer, std::move(fakePlayerList));
    if (unnamedList.size())
        playerList.emplace_back(PlayerCategory::Unnamed, std::move(unnamedList));
    return playerList;
}

std::unique_ptr<CompoundTag> CheckBagManager::getBackupBag(Player* player)
{
    if (auto log = tryGetLog(player)) {
        std::unique_ptr<CompoundTag> tag = {};
        log->mBackup.swap(tag);
        mCheckBagLogMap.erase(player->getUuid());
        updateIsFree();
        return tag;
    }
    else {
        auto path = getBackupPath(player);
        auto bin = ReadAllFile(path, Config::BackupDataType == NbtDataType::Binary);
        if (bin.has_value())
            return PlayerDataHelper::deserializeNbt(std::move(bin.value()), Config::BackupDataType);
        return {};
    }
}

CheckBagManager::Result CheckBagManager::removePlayerData(ServerPlayer* player)
{
    mIsFree = false;
    auto uuid = player->getUuid();
    mRemoveRequsets.emplace(uuid, player->getUniqueID().id);
    return Result::Request;
}

CheckBagManager::Result CheckBagManager::removePlayerData(mce::UUID const& uuid)
{
    if (!uuid)
        return Result::Error;
    if (auto player = getPlayer(uuid)) {
        mRemoveRequsets.emplace(uuid.asString(), player->getUniqueID().id);
        mIsFree = false;
        return Result::Success;
    }
    if (PlayerDataHelper::removeData(uuid)) {
        mUuidNameMap.erase(uuid.asString());
        return Result::Success;
    }
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::setCheckBagLog(Player* player, mce::UUID const& target, CompoundTag& tag)
{
    if (auto log = tryGetLog(player)) {
        log->mTarget = target;
        return Result::Success;
    }
    auto&& data = PlayerDataHelper::serializeNbt(tag.clone(), Config::BackupDataType);
    if (WriteAllFile(getBackupPath(player), data, true)) {
        mCheckBagLogMap.emplace(player->getUuid(), CheckBagLog(target, tag.clone()));
        return Result::Success;
    }
    return Result::BackupError;
}

CheckBagManager::Result CheckBagManager::overwriteBagData(Player* player, CheckBagLog const& log) {
    auto targetPlayer = log.getTarget();
    auto data = player->getNbt();
    if (targetPlayer) {
        if (PlayerDataHelper::setPlayerBag(targetPlayer, *data))
            return Result::Success;
        return Result::Error;
    }
    if (PlayerDataHelper::writePlayerBag(log.mTarget, *data))
        return Result::Success;
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::restoreBagData(Player* player)
{
    if (Config::PacketMode) {
        player->refreshInventory();
        return Result::Success;
    }
    else {
        auto backupPath = getBackupPath(player);
        auto backupTag = getBackupBag(player);
        if (!backupTag)
            return Result::BackupNotFound;
        PlayerDataHelper::setPlayerBag(player, *backupTag);
        player->refreshInventory();
        std::filesystem::remove(backupPath);
        return Result::Success;
    }
}

CheckBagManager::Result CheckBagManager::setBagData(Player* player, mce::UUID const& uuid, std::unique_ptr<CompoundTag> targetTag)
{
    if (Config::PacketMode) {
        // sendBagData();
        return Result::Error;
    }
    else {
        auto playerTag = player->getNbt();
        auto res = setCheckBagLog(player, uuid, *playerTag);

        if (res == Result::Success) {
            auto res = PlayerDataHelper::changeBagTag(*playerTag, *targetTag);
            res = res && player->setNbt(playerTag.get());
            res = res && player->refreshInventory();
            if (res)
                return Result::Success;
            return Result::Error;
        };
        return res;
    }
}

CheckBagManager::Result CheckBagManager::stopCheckBag(Player* player)
{
    if (!tryGetLog(player))
        return Result::NotStart;
    auto rtn = restoreBagData(player);
    updateIsFree();
    return rtn;

}

CheckBagManager::Result CheckBagManager::startCheckBag(Player* player, Player* target)
{
    mIsFree = false;
    // TODO 
    auto uuid = target->getUuid();
    return setBagData(player, mce::UUID::fromString(uuid), target->getNbt());
}

CheckBagManager::Result CheckBagManager::startCheckBag(Player* player, mce::UUID const& uuid)
{
    mIsFree = false;
    if (auto target = getPlayer(uuid))
        return startCheckBag(player, target);
    auto targetTag = PlayerDataHelper::getPlayerTag(uuid);
    if (!targetTag)
        return Result::TargetNotExist;
    return setBagData(player, uuid, std::move(targetTag));
}

CheckBagManager::Result CheckBagManager::overwriteData(Player* player)
{
    auto log = tryGetLog(player);
    if (!log)
        return Result::NotStart;
    auto rtn = overwriteBagData(player, *log);
    restoreBagData(player);
    updateIsFree();
    return rtn;
}

CheckBagManager::Result CheckBagManager::exportData(mce::UUID const& uuid, NbtDataType type = NbtDataType::Snbt) {
    if (!uuid)
        return Result::Error;
    std::string data;
    if (!getPlayer(uuid) && type == NbtDataType::Binary) {
        data = PlayerDataHelper::getPlayerData(uuid);
    }
    else {
        std::unique_ptr<CompoundTag> tag = PlayerDataHelper::getExpectedPlayerTag(uuid);
        if (!tag)
            return Result::Error;
        data = PlayerDataHelper::serializeNbt(std::move(tag), type);
    }

    if (data.empty())
        return Result::Error;

    auto playerName = PlayerInfo::fromUUID(uuid.asString());
    auto idsTag = PlayerDataHelper::getPlayerIdsTag(uuid);
    nlohmann::json playerInfo;
    playerInfo["name"] = playerName;
    playerInfo["uuid"] = uuid.asString();
    for (auto& [type, idTag] : *idsTag) {
        playerInfo[type] = const_cast<CompoundTagVariant&>(idTag).asStringTag()->value();
    }
    auto infoStr = playerInfo.dump(4);

    auto dataPath = getExportPath(uuid, type);
    std::filesystem::path infoPath(dataPath + ".json");
    if (WriteAllFile(dataPath, data, true) && WriteAllFile(infoPath.string(), infoStr, false))
        return Result::Success;
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::exportData(std::string const& nameOrUuid, NbtDataType type = NbtDataType::Snbt)
{
    if (auto uuid = fromNameOrUuid(nameOrUuid))
        return exportData(uuid, type);
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::importData(mce::UUID const& uuid, std::string filePath, bool isBagOnly) {
    if (!uuid || filePath.empty())
        return Result::Error;
    auto suffix = filePath.substr(filePath.find_last_of('.') + 1);
    auto newTag = PlayerDataHelper::readTagFile(filePath, fromSuffix(suffix));
    if (!newTag)
        return Result::Error;
    if (isBagOnly) {
        if (auto player = getPlayer(uuid)) {
            PlayerDataHelper::setPlayerBag(player, *newTag);
        }
        else {
            auto oldTag = PlayerDataHelper::getExpectedPlayerTag(uuid);
            if (!oldTag)
                return Result::Error;
            if (PlayerDataHelper::writePlayerBag(uuid, *newTag))
                return Result::Success;
            return Result::Error;
        }
    }
    else {
        if (getPlayer(uuid))
            return Result::Error;
        if (PlayerDataHelper::writePlayerData(uuid, *newTag))
            return Result::Success;
    }
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::importData(std::string const& nameOrUuid, std::string filePath, bool isBagOnly)
{
    return Result::Error;
    if (auto uuid = fromNameOrUuid(nameOrUuid))
        return importData(uuid, filePath, isBagOnly);
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::importNewData(std::string filePath) {
    if (!std::filesystem::exists(filePath + ".json")) {
        logger.error("Failed to get player info file，file {} not exist", filePath + ".json");
        return Result::InfoDataNotFound;
    }
    auto infoData = ReadAllFile(filePath + ".json");

    auto playerIds = nlohmann::json::parse(infoData.value_or(""));
    auto idsTag = CompoundTag::create();
    std::string name;
    std::string suuid;
    for (auto& [key, val] : playerIds.items()) {
        std::string id = val.get<std::string>();
        logger.error("{}: {},", key, id);
        if (key == "name") {
            name = id;
            continue;
        }
        if (key == "uuid") {
            suuid = id;
            continue;
        }
        idsTag->putString(key, id);
    }
    if (suuid.empty()){
        suuid = idsTag->getString("MsaId");
        if (suuid.empty() && !Config::MsaIdOnly)
            suuid = idsTag->getString("SelfSignedId");
    }
    if (suuid.empty())
        return Result::Error;

    auto suffix = filePath.substr(filePath.find_last_of('.') + 1);
    auto type = fromSuffix(suffix);
    if (type != NbtDataType::Binary && type != NbtDataType::Snbt)
        return Result::DataTypeNotSupported;
    auto data = ReadAllFile(filePath, NbtDataType::Binary == type);
    auto tag = PlayerDataHelper::deserializeNbt(data.value_or(""));
    if (!tag || idsTag->isEmpty()) {
        return Result::Error;
    }
    if (PlayerDataHelper::writeNewPlayerData(std::move(idsTag), std::move(tag))) {
        bool isFakePlayer = PlayerDataHelper::isFakePlayer_ddf8196(suuid);
        mUuidNameMap.emplace(suuid, std::pair{(name.empty() ? suuid : name), isFakePlayer});
        return Result::Success;
    }
    return Result::Error;
}

size_t CheckBagManager::exportAllData(NbtDataType type)
{
    size_t count = 0;
    for (auto& suuid : getPlayerList()) {
        auto result = exportData(suuid, type);
        if (result == CheckBagManager::Result::Success)
            count++;
        else {
            logger.warn("导出 {} 数据失败，原因：{}", suuid, CheckBagManager::getResultString(result));
        }
    }
    return count;
}

TClasslessInstanceHook(void, "?_onPlayerLeft@ServerNetworkHandler@@AEAAXPEAVServerPlayer@@_N@Z",
    ServerPlayer* sp, bool a3)
{
    if (CheckBagManager::mIsFree)
        return original(this, sp, a3);

    auto& manager = CheckBagManager::getManager();
    // 保存玩家数据前
    manager.beforePlayerLeave(sp);
    original(this, sp, a3);
    // 玩家数据保存后
    manager.afterPlayerLeave(sp);
}
