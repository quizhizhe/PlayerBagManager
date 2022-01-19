#pragma once

#define CheckBagMgr CheckBagManager::getManager()

inline class Player* getPlayer(class mce::UUID const& a0) {
    class Player* (Level:: * rv)(class mce::UUID const&);
    *((void**)&rv) = dlsym("?getPlayer@Level@@UEBAPEAVPlayer@@AEBVUUID@mce@@@Z");
    return (Global<Level>->*rv)(std::forward<class mce::UUID const&>(a0));
}
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
    class UuidNameMap {
        // <UUID, name>
        std::unordered_map<std::string, std::string> mUuidNameMap;
        // <name, UUID>
        std::unordered_map<std::string, std::string> mNameUuidMap;
        // <UUID, isFakePlayer>
        std::unordered_map<std::string, bool> mFakePlayerMap;
    public:
        UuidNameMap() {};
        UuidNameMap(std::unordered_map<std::string, std::string>&& uuidNameMap);
        inline std::string const& tryGetUuid(std::string const& name) {
            auto iter = mNameUuidMap.find(name);
            if (iter != mNameUuidMap.end())
                return iter->second;
            return "";
        }
        inline std::string const& tryGetName(std::string const& uuid) {
            auto iter = mUuidNameMap.find(uuid);
            if (iter != mUuidNameMap.end())
                return iter->second;
            return "";
        }
        inline std::string const& tryGetName(mce::UUID const& uuid) {
            auto iter = mUuidNameMap.find(uuid.asString());
            if (iter != mUuidNameMap.end())
                return iter->second;
            return "";
        }
        bool emplace(std::string const& uuid, std::string const& name);
        inline void clear() {
            mUuidNameMap.clear();
            mNameUuidMap.clear();
        }
        inline auto size() {
            return mUuidNameMap.size();
        }
        inline auto begin() {
            return mUuidNameMap.begin();
        }
        inline auto end() {
            return mUuidNameMap.end();
        }
        inline auto find(std::string const& key) {
            return mNameUuidMap.find(key);
        }
        inline bool isFakePlayer(std::string const& uuid) {
            auto iter = mFakePlayerMap.find(uuid);
            return iter != mFakePlayerMap.end() && iter->second;
        }
    };
    UuidNameMap mUuidNameMap;
    // <UuidString, ActorUniqueID>
    std::unordered_map<std::string, __int64> mRemoveRequsets;
    // <UuidString, CheckBagLog>
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
    };

    static bool mIsFree;
    static CheckBagManager& getManager();

    static std::string getSuffix(NbtDataType type);
    static NbtDataType fromSuffix(std::string_view suffix);
    inline static std::string getResultString(Result result) {
        switch (result)
        {
        case CheckBagManager::Result::Success:
            return "成功";
        case CheckBagManager::Result::Error:
            return "未知错误";
        case CheckBagManager::Result::Request:
            return "操作冲突，改为请求操作";
        case CheckBagManager::Result::NotStart:
            return "未开始";
        case CheckBagManager::Result::BackupError:
            return "备份失败";
        case CheckBagManager::Result::BackupNotFound:
            return "未找到备份";
        case CheckBagManager::Result::TargetNotExist:
            return "目标不存在";
        default:
            return "未知错误";
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
        return mCheckBagLogMap.find(uuid) == mCheckBagLogMap.end();
    }
    inline std::string getBackupPath(Player* player) {
        auto realName = player->getRealName();
        auto path = std::filesystem::path(Config::BackupDirectory);
        path.append(realName + ".nbt");
        return path.string();
    }
    inline std::string getExportPath(std::string const& name, std::string const& suffix) {
        auto path = std::filesystem::path(Config::ExportDirectory);
        path.append(name + "." + suffix);
        return path.string();
    }
    std::vector<std::string> getPlayerList();
    std::vector<std::string> getPlayerList(PlayerCategory category);
    std::vector<std::pair<PlayerCategory, std::vector<std::string>>> getClassifiedPlayerList();
    std::unique_ptr<CompoundTag> getBackupBag(Player* player);

    Result removePlayerData(ServerPlayer* player);
    Result removePlayerData(mce::UUID const& uuid);
    Result backupData(Player* player, mce::UUID const& target, CompoundTag& tag);
    Result overwriteBagData(Player* player, CheckBagLog const& log);
    Result restoreBagData(Player* player);
    Result setBagData(Player* player, mce::UUID const& uuid, std::unique_ptr<CompoundTag> targetTag);
    Result stopCheckBag(Player* player);
    Result startCheckBag(Player* player, Player* target);
    Result startCheckBag(Player* player, mce::UUID const& uuid);
    Result overwriteData(Player* player);
    Result exportData(mce::UUID const& uuid, NbtDataType type);
    Result exportData(std::string const& name, NbtDataType type);

    // Callback will be ignore if player cancel the form
    bool sendPlayerListForm(Player* player, std::string const& title, std::string const& content,
        std::function<void(Player* player, mce::UUID const& uuid)>&& callback);
    // Callback will be ignore if player cancel the form
    bool sendDataTypeForm(Player* player, std::string const& title, std::string const& content,
        std::function<void(Player* player, NbtDataType type)>&& callback);
};

