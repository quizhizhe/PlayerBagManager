#pragma once
#include "PlayerDataHelper.h"
// #define CBMgr CheckBagManager::getManager()
// // BidirectionalUnorderedMap<UuidString, name>


class ServerPlayer;

namespace player_bag_manager::check_bag_manager {

class CheckBagManager {
    CheckBagManager();
    CheckBagManager(CheckBagManager& manager) = delete;
    struct CheckBagLog {
        mce::UUID                    mTarget;
        std::unique_ptr<CompoundTag> mBackup;
        PlayerCategory               mCategory;

        CheckBagLog(
            mce::UUID const&             target,
            std::unique_ptr<CompoundTag> backup,
            PlayerCategory               category = PlayerCategory::All
        )
        : mBackup(std::move(backup)),
          mTarget(target),
          mCategory(category) {}
        // CheckBagLog(Player* target, std::unique_ptr<CompoundTag> backup, PlayerCategory category =
        // PlayerCategory::All) : mBackup(std::move(backup)) {
        //     auto uuid = target->getUuid();
        //     mTarget = mce::UUID::fromString(uuid);
        // }
    };
    // <UUID, <Name, IsFakePlayer>>
    std::unordered_map<mce::UUID, std::pair<std::string, bool>> mUuidNameMap;
    // <UUID, ActorUniqueID>
    std::unordered_map<mce::UUID, __int64> mRemoveRequsets;
    // <UUID, CheckBagLog>
    std::unordered_map<mce::UUID, CheckBagLog> mCheckBagLogMap;

    inline CheckBagLog* tryGetLog(mce::UUID const& uuid) {
        auto logIter = mCheckBagLogMap.find(uuid);
        if (logIter == mCheckBagLogMap.end()) return nullptr;
        return &logIter->second;
    }
    // inline CheckBagLog* tryGetLog(Player* player) {
    //     return tryGetLog(player->getUuid());
    // }
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

    static bool             mIsFree;
    static CheckBagManager& getManager();

    static std::string getSuffix(NbtDataType type);
    static NbtDataType fromSuffix(std::string const& suffix);
    static std::string getResultI18(Result result);


    void updateIsFree() { mIsFree = mRemoveRequsets.size() + mCheckBagLogMap.size(); }

    void beforePlayerLeave(Player* player);
    void afterPlayerLeave(Player* player);
    void afterPlayerJoin(Player* player);

    bool isCheckingBag(Player* player);

    mce::UUID tryGetTargetUuid(Player* player);

    std::string getNameOrUuid(mce::UUID const& uuid);

    static mce::UUID getUUIDFromName(std::string const& name);

    static std::string getBackupPath(Player* player);

    std::string getExportPath(mce::UUID const& uuid, NbtDataType type);

    std::vector<std::string>                                         getPlayerList();
    std::vector<std::string>                                         getPlayerList(PlayerCategory category);
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> getClassifiedPlayerList();
    std::unique_ptr<CompoundTag>                                     getBackupBag(Player* player);

    Result removePlayerData(Player* player);
    Result removePlayerData(mce::UUID const& uuid);
    Result setCheckBagLog(Player* player, mce::UUID const& target, CompoundTag& tag);
    Result overwriteBagData(Player* player, CheckBagLog const& log);
    Result restoreBagData(Player* player);
    Result setBagData(Player* player, mce::UUID const& uuid, std::unique_ptr<CompoundTag> targetTag);
    Result stopCheckBag(Player* player);
    Result startCheckBag(Player* player, Player* target);
    Result startCheckBag(Player* player, mce::UUID const& uuid);
    Result checkNext(Player* player);
    Result checkPrevious(Player* player);
    Result overwriteData(Player* player);
    Result exportData(mce::UUID const& uuid, NbtDataType type);
    Result exportData(std::string const& nameOrUuid, NbtDataType type);
    Result importData(mce::UUID const& uuid, std::string filePath, bool isBagOnly);
    Result importData(std::string const& nameOrUuid, std::string filePath, bool isBagOnly);
    Result importNewData(std::string filePath);
    size_t exportAllData(NbtDataType type);
};

} // namespace player_bag_manager::check_bag_manager