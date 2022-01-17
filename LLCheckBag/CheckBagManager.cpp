#include "pch.h"
#include "CheckBagManager.h"
#include "PlayerDataHelper.h"
#include <PlayerInfoAPI.h>

// Test
#if false
LARGE_INTEGER freq_;
auto INITPERFORMANCEFREQUENCY = QueryPerformanceFrequency(&freq_);
LARGE_INTEGER begin_time;
LARGE_INTEGER end_time;
inline double ns_time() {
    return (end_time.QuadPart - begin_time.QuadPart) * 1000000.0 / freq_.QuadPart;
}
#define TestLogTime(func, ...)\
QueryPerformanceCounter(&begin_time);\
func(__VA_ARGS__);\
QueryPerformanceCounter(&end_time);\
logger.warn("  {}\t time: {}ns", #func, ns_time());

void testTime() {
    TestLogTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestLogTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestLogTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
    TestLogTime(PlayerDataHelper::getAllUuid, !Config::MsaIdOnly);
}
#else
#define testTime() ((void)0)
#endif

bool CheckBagManager::mIsFree = true;

CheckBagManager::CheckBagManager() {
    testTime();
    mUuidList = PlayerDataHelper::getAllUuid(!Config::MsaIdOnly);
};

CheckBagManager& CheckBagManager::getManager()
{
    static CheckBagManager manager;
    return manager;
}

std::string CheckBagManager::getSuffix(DataType type)
{
    std::string suffix;
    switch (type)
    {
    case CheckBagManager::DataType::Snbt:
        suffix = "snbt";
        break;
    case CheckBagManager::DataType::Binary:
        suffix = "nbt";
        break;
    case CheckBagManager::DataType::Json:
        suffix = "json";
        break;
    default:
        break;
    }
    return suffix;
}

CheckBagManager::DataType CheckBagManager::fromSuffix(std::string_view suffix)
{
    if (suffix == "snbt")
        return DataType::Snbt;
    if (suffix == "nbt")
        return DataType::Binary;
    if (suffix == "json")
        return DataType::Json;
    return DataType::Unknown;
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

std::vector<std::string> CheckBagManager::getPlayerList() {
    std::vector<std::string> playerList;
    playerList.resize(mUuidList.size());
    size_t index = 0;
    size_t rindex = mUuidList.size() - 1;
    for (auto& uuid : mUuidList) {
        auto name = PlayerInfo::fromUUID(uuid);
        if (name.empty())
            playerList[rindex--] = uuid;
        else
            playerList[index++] = name;
    }
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
            return CompoundTag::fromBinaryNBT((void*)bin.value().c_str(), bin.value().size());
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
            PlayerDataHelper::changeBagTag(*playerTag, *targetTag);
            player->setNbt(playerTag.get());
            player->refreshInventory();
            return Result::Success;
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
    return setBagData(player, uuid, PlayerDataHelper::getPlayerData(uuid));
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

CheckBagManager::Result CheckBagManager::exportData(std::string const& name, DataType type)
{
    std::string suffix = getSuffix(type);
    auto path = getExportPath(name, suffix);
    auto uuid = mce::UUID::fromString(name);
    std::unique_ptr<CompoundTag> tag;
    if (uuid) {
        tag = PlayerDataHelper::getPlayerData(uuid);
    }
    else {
        auto suuid = PlayerInfo::getUUID(name);
        auto uuid = mce::UUID::fromString(suuid);
        tag = PlayerDataHelper::getPlayerData(uuid);
    }
    if (WriteAllFile(path, tag->toBinaryNBT(), true))
        return Result::Success;
    return Result::Error;
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