#pragma once
#include <MC/Level.hpp>


inline bool nameSortFunc(std::string const& left, std::string const& right) {
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
inline class Player* getPlayer(class mce::UUID const& a0) {
    class Player* (Level:: * rv)(class mce::UUID const&);
    *((void**)&rv) = dlsym("?getPlayer@Level@@UEBAPEAVPlayer@@AEBVUUID@mce@@@Z");
    return (Global<Level>->*rv)(std::forward<class mce::UUID const&>(a0));
}

namespace PlayerDataHelper {
    void forEachUuid(bool allowSelfSignedId, std::function<void(std::string_view const& uuid)> callback);
    std::vector<string> getAllUuid(bool allowSelfSignedId);
    std::unique_ptr<CompoundTag> getPlayerIdsTag(mce::UUID const& uuid);
    bool removeData(mce::UUID const& uuid);
    std::string getServerId(mce::UUID const& uuid);
    std::string getPlayerData(mce::UUID const& uuid);
    std::unique_ptr<CompoundTag> getPlayerTag(mce::UUID const& uuid);
    std::unique_ptr<CompoundTag> getExpectedPlayerTag(mce::UUID const& uuid);
    bool writePlayerData(mce::UUID const& uuid, CompoundTag& data);
    bool changeBagTag(CompoundTag& dst, CompoundTag& src);
    bool setPlayerBag(Player* player, CompoundTag& data);
    bool writePlayerBag(mce::UUID const& uuid, CompoundTag& data);
    std::string serializeNbt(std::unique_ptr<CompoundTag> tag, NbtDataType type = NbtDataType::Binary);
    std::unique_ptr<CompoundTag> deserializeNbt(std::string const& data, NbtDataType type = NbtDataType::Binary);
    bool isFakePlayer_ddf8196(mce::UUID const& uuid);
    bool isFakePlayer_ddf8196(std::string const& suuid);
    std::unique_ptr<CompoundTag> readTagFile(std::string const& path, NbtDataType type = NbtDataType::Binary);
    bool writeNewPlayerData(std::unique_ptr<CompoundTag> idsTag, std::unique_ptr<CompoundTag> dataTag);
}
