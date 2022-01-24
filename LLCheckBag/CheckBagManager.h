#pragma once
#include "PlayerDataHelper.h"
#define CBMgr CheckBagManager::getManager()
// BidirectionalUnorderedMap<UuidString, name>
class CheckBagManager
{
    CheckBagManager();
    CheckBagManager(CheckBagManager& manager) = delete;
    struct CheckBagLog {
        mce::UUID mTarget;
        std::unique_ptr<CompoundTag> mBackup;

        CheckBagLog(mce::UUID const& target, std::unique_ptr<CompoundTag> backup)
            :mBackup(std::move(backup)),
            mTarget(target) {
        }
        CheckBagLog(Player* target, std::unique_ptr<CompoundTag> backup) : mBackup(std::move(backup)) {
            auto uuid = target->getUuid();
            mTarget = mce::UUID::fromString(uuid);
        }

        inline Player* getTarget() const {
            return getPlayer(mTarget);
        }

    };
    // <UUID, <Name, IsFakePlayer>>
    std::unordered_map<std::string, std::pair<std::string, bool>> mUuidNameMap;
    // <UUID, ActorUniqueID>
    std::unordered_map<std::string, __int64> mRemoveRequsets;
    // <UUID, CheckBagLog>
    std::unordered_map<std::string, CheckBagLog> mCheckBagLogMap;

    inline CheckBagLog* tryGetLog(std::string const& uuid) {
        auto logIter = mCheckBagLogMap.find(uuid);
        if (logIter == mCheckBagLogMap.end())
            return nullptr;
        return &logIter->second;
    }
    inline CheckBagLog* tryGetLog(Player* player) {
        return tryGetLog(player->getUuid());
    }
    void initUuidNameMap();
public:
    enum class Result : int {
        Success,
        Error,
        Request,
        NotStart,
        BackupError,
        BackupNotFound,
        TargetNotExist,
        InfoDataNotFound,
        DataTypeNotSupported,
    };

    static bool mIsFree;
    static CheckBagManager& getManager();

    static std::string getSuffix(NbtDataType type);
    static NbtDataType fromSuffix(std::string const& suffix);
    inline static std::string getResultString(Result result) {
        switch (result)
        {
        case CheckBagManager::Result::Success:
            return tr("manager.result.success");
        case CheckBagManager::Result::Error:
            return tr("manager.result.error");
        case CheckBagManager::Result::Request:
            return tr("manager.result.player_online");
        case CheckBagManager::Result::NotStart:
            return tr("manager.result.not_start");
        case CheckBagManager::Result::BackupError:
            return tr("manager.result.backup_failed");
        case CheckBagManager::Result::BackupNotFound:
            return tr("manager.result.backup_not_found");
        case CheckBagManager::Result::TargetNotExist:
            return tr("manager.result.target_not_exists");
        case CheckBagManager::Result::InfoDataNotFound:
            return tr("manager.result.info_data_not_found");
        case CheckBagManager::Result::DataTypeNotSupported:
            return tr("manager.result.data_type_not_supported");
        default:
            return tr("manager.result.unknown_error");
        }
    }


    inline void updateIsFree() {
        mIsFree = mRemoveRequsets.size() + mCheckBagLogMap.size();
    }

    void beforePlayerLeave(ServerPlayer* player);
    void afterPlayerLeave(ServerPlayer* player);
    void afterPlayerJoin(ServerPlayer* player);

    inline bool isCheckingBag(Player* player) {
        auto uuid = player->getUuid();
        return mCheckBagLogMap.find(uuid) != mCheckBagLogMap.end();
    }
    inline mce::UUID tryGetTargetUuid(Player* player) {
        auto log = tryGetLog(player);
        if (log)
            return log->mTarget;
        return mce::UUID::fromString("");
    }
    inline std::string getNameOrUuid(mce::UUID const& uuid) {
        auto suuid = uuid.asString();
        auto iter = mUuidNameMap.find(suuid);
        if (iter != mUuidNameMap.end())
            return iter->second.first;
        return suuid;
    }
    static mce::UUID fromNameOrUuid(std::string const& nameOrUuid);
    inline static std::string getBackupPath(Player* player) {
        auto realName = player->getRealName();
        auto path = std::filesystem::path(Config::BackupDirectory);
        path.append(realName + "." + getSuffix(Config::BackupDataType));
        return path.string();
    }
    inline std::string getExportPath(mce::UUID const& uuid, NbtDataType type) {
        auto fileName = getNameOrUuid(uuid);
        std::string suffix = getSuffix(type);
        auto path = std::filesystem::path(Config::ExportDirectory);
        path.append(fileName + "." + suffix);
        return path.string();
    }

    std::vector<std::string> getPlayerList();
    std::vector<std::string> getPlayerList(PlayerCategory category);
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> getClassifiedPlayerList();
    std::unique_ptr<CompoundTag> getBackupBag(Player* player);

    Result removePlayerData(ServerPlayer* player);
    Result removePlayerData(mce::UUID const& uuid);
    Result setCheckBagLog(Player* player, mce::UUID const& target, CompoundTag& tag);
    Result overwriteBagData(Player* player, CheckBagLog const& log);
    Result restoreBagData(Player* player);
    Result setBagData(Player* player, mce::UUID const& uuid, std::unique_ptr<CompoundTag> targetTag);
    Result stopCheckBag(Player* player);
    Result startCheckBag(Player* player, Player* target);
    Result startCheckBag(Player* player, mce::UUID const& uuid);
    Result overwriteData(Player* player);
    Result exportData(mce::UUID const& uuid, NbtDataType type);
    Result exportData(std::string const& nameOrUuid, NbtDataType type);
    Result importData(mce::UUID const& uuid, std::string filePath, bool isBagOnly);
    Result importData(std::string const& nameOrUuid, std::string filePath, bool isBagOnly);
    Result importNewData(std::string filePath);
    size_t exportAllData(NbtDataType type);

};

