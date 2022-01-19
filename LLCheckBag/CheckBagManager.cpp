#include "pch.h"
#include "CheckBagManager.h"
#include "PlayerDataHelper.h"
#include <PlayerInfoAPI.h>
#include <FormUI.h>

bool stringSortFunc(std::string const& left, std::string const& right) {
    size_t maxSize = std::max(left.size(), right.size());

    for (size_t i = 0; i < maxSize; i++)
    {
        auto c1 = left[i];
        auto c2 = right[i];
        if (c1 == c2)
            continue;
        auto tmp = tolower(c1) - tolower(c2);
        if (tmp == 0)
            return c1 > c2;
        return tmp < 0;
    }
    return right.size() - left.size();
}

bool CheckBagManager::mIsFree = true;

CheckBagManager::CheckBagManager() {
    TestFuncTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestFuncTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    mUuidList = PlayerDataHelper::getAllUuid(!Config::MsaIdOnly);
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

NbtDataType CheckBagManager::fromSuffix(std::string_view suffix)
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

}

void CheckBagManager::afterPlayerLeave(ServerPlayer* player)

{
    if (mRemoveRequsets.empty())
        return;
    auto uuidStr = player->getUuid();
    auto uuid = mce::UUID::fromString(uuidStr);
    auto uuidIter = mRemoveRequsets.find(uuidStr);
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
inline std::string getDisplayString(std::string const& suuid) {
    auto name = PlayerInfo::fromUUID(suuid);
    if (name.empty())
        return suuid;
    else
        return name;
}

std::vector<std::string> CheckBagManager::getPlayerList() {
    std::vector<std::string> playerList;
    playerList.resize(mUuidList.size());
    size_t index = 0;
    size_t rindex = mUuidList.size() - 1;
    for (auto& uuid : mUuidList) {
        TestFuncTime(PlayerInfo::fromUUID, uuid);
        auto name = PlayerInfo::fromUUID(uuid);
        if (name.empty())
            playerList[rindex--] = uuid;
        else
            playerList[index++] = name;
    }
    std::sort(playerList.begin(), playerList.begin() + index, stringSortFunc);
    std::sort(playerList.begin() + index, playerList.end());
    return playerList;
}

std::vector<std::string> CheckBagManager::getPlayerList(PlayerCategory category) {
    if (category == PlayerCategory::All)
        return getPlayerList();
    std::vector<std::string> playerList;
    size_t index = 0;
    size_t rindex = mUuidList.size() - 1;
    for (auto& suuid : mUuidList) {
        auto uuid = mce::UUID::fromString(suuid);
        TestFuncTime(PlayerDataHelper::isFakePlayer_ddf8196, uuid);
        if (PlayerDataHelper::isFakePlayer_ddf8196(uuid)) {
            if (category != PlayerCategory::FakePlayer)
                continue;
            auto name = PlayerInfo::fromUUID(suuid);
            if (name.empty())
                playerList.push_back(suuid);
            else
                playerList.emplace_back(name);
            continue;
        }
        else {
            auto name = PlayerInfo::fromUUID(suuid);
            if (name.empty()) {
                if (category == PlayerCategory::Unnamed)
                    playerList.push_back(suuid);
            }
            else if(category == PlayerCategory::Normal)
                playerList.emplace_back(name);
            continue;
        }
    }
    TestFuncTime(std::sort, playerList.begin(), playerList.end(), stringSortFunc);
    std::sort(playerList.begin(), playerList.end(), stringSortFunc);
    return playerList;
}

std::vector<std::pair<PlayerCategory, std::vector<std::string>>> CheckBagManager::getClassifiedPlayerList() {
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> playerList;
    std::vector<std::string> normalList;
    std::vector<std::string> fakePlayerList;
    std::vector<std::string> unnamedFakePlayerList;
    std::vector<std::string> unnamedList;
    for (auto& suuid : mUuidList) {
        auto uuid = mce::UUID::fromString(suuid);
        if (PlayerDataHelper::isFakePlayer_ddf8196(uuid)) {
            auto name = PlayerInfo::fromUUID(suuid);
            if (name.empty())
                fakePlayerList.push_back(suuid);
            else
                unnamedFakePlayerList.emplace_back(name);
            continue;
        }
        else {
            auto name = PlayerInfo::fromUUID(suuid);
            if (name.empty())
                unnamedList.push_back(suuid);
            else
                normalList.emplace_back(name);
            continue;

        }
    }

    std::sort(normalList.begin(), normalList.end(), stringSortFunc);
    std::sort(fakePlayerList.begin(), fakePlayerList.end(), stringSortFunc);
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
    auto tagIter = mCheckBagLog.find(player->getRealName());
    if (tagIter != mCheckBagLog.end()) {
        std::unique_ptr<CompoundTag> tag = {};
        tagIter->second.mBackup.swap(tag);
        mCheckBagLog.erase(tagIter);
        updateIsFree();
        return tag;
    }
    else {
        auto path = getBackupPath(player);
        auto bin = ReadAllFile(path);
        if (bin.has_value())
            return PlayerDataHelper::deserializeNbt(bin.value(), NbtDataType::Binary);
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
    if (PlayerDataHelper::removeData(uuid))
        return Result::Success;
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::backupData(Player* player, mce::UUID const& target, CompoundTag& tag)
{
    auto uuid = player->getUuid();
    auto logIter = mCheckBagLog.find(uuid);
    if (logIter != mCheckBagLog.end()) {
        logIter->second.mTarget = target;
        //tag.deepCopy(*logIter->second.mBackup);
        return Result::Success;
    }
    if (WriteAllFile(getBackupPath(player), tag.toBinaryNBT(), true)) {
        mCheckBagLog.emplace(player->getRealName(), CheckBagLog(target, tag.clone()));
        return Result::Success;
    }
    return Result::BackupError;
}

CheckBagManager::Result CheckBagManager::overwriteBagData(Player* player, CheckBagLog const& log) {
    auto target = log.getTarget();
    auto data = player->getNbt();
    if (target) {
        if (PlayerDataHelper::setPlayerBag(target, *data))
            return Result::Success;
        return Result::Error;
    }
    if(PlayerDataHelper::writePlayerBag(log.mTarget, *data))
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
        std::wstring wPath(backupPath.begin(), backupPath.end());
        DeleteFile(wPath.c_str());
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
        auto res = backupData(player, uuid, *playerTag);

        if (res == Result::Success) {
            auto res = PlayerDataHelper::changeBagTag(*playerTag, *targetTag);
            res = res && player->setNbt(playerTag.get());
            res = res && player->refreshInventory();
            if(res)
                return Result::Success;
            return Result::Error;
        };
        return res;
    }
}

CheckBagManager::Result CheckBagManager::stopCheckBag(Player* player)
{
    if (mCheckBagLog.find(player->getRealName()) == mCheckBagLog.end())
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
    auto targetTag = PlayerDataHelper::getPlayerData(uuid);
    if (!targetTag)
        return Result::TargetNotExist;
    return setBagData(player, uuid, std::move(targetTag));
}

CheckBagManager::Result CheckBagManager::overwriteData(Player* player)
{
    auto logIter = mCheckBagLog.find(player->getUuid());
    if (logIter == mCheckBagLog.end())
        return Result::NotStart;
    auto rtn = overwriteBagData(player, logIter->second);
    restoreBagData(player);
    updateIsFree();
    return rtn;
}

CheckBagManager::Result CheckBagManager::exportData(mce::UUID const& uuid, NbtDataType type = NbtDataType::Snbt) {
    if(!uuid)
        return Result::Error;
    std::string suffix = getSuffix(type);
    auto path = getExportPath(uuid.asString(), suffix);
    std::unique_ptr<CompoundTag> tag = PlayerDataHelper::getPlayerData(uuid);
    nlohmann::json playerInfo;
    auto playerName = PlayerInfo::fromUUID(uuid.asString());
    playerInfo["name"] = playerName;
    playerInfo["uuid"] = uuid.asString();
    playerInfo["ServerId"] = PlayerDataHelper::getServerId(uuid);
    auto infoStr = playerInfo.dump(4);
    std::filesystem::path infoPath(path);
    auto fileName = playerName.empty() ? uuid.asString() : playerName;
    fileName += "_info.json";
    infoPath.remove_filename().append(fileName);
    std::string data = PlayerDataHelper::serializeNbt(std::move(tag), type);
    if (WriteAllFile(path, data, true) && WriteAllFile(infoPath.string(), infoStr, false))
        return Result::Success;
    return Result::Error;
}

CheckBagManager::Result CheckBagManager::exportData(std::string const& name, NbtDataType type = NbtDataType::Snbt)
{
    auto suuid = PlayerInfo::getUUID(name);
    if (suuid.empty())
        suuid = name;
    auto uuid = mce::UUID::fromString(suuid);
    if (!uuid)
        return Result::Error;
    return exportData(uuid, type);
}

TClasslessInstanceHook(void, "?_onPlayerLeft@ServerNetworkHandler@@AEAAXPEAVServerPlayer@@_N@Z",
    ServerPlayer* sp, bool a3)
{
    if(!CheckBagManager::mIsFree)
        return original(this, sp, a3);

    auto& manager = CheckBagManager::getManager();
    // 保存玩家数据前
    manager.beforePlayerLeave(sp);
    original(this, sp, a3);
    // 玩家数据保存后
    manager.afterPlayerLeave(sp);
}