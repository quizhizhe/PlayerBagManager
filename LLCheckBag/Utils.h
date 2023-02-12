#pragma once
#include <Global.h>
#include <PlayerInfoAPI.h>
#include <mc/Level.hpp>
#include <mc/Player.hpp>

inline mce::UUID UuidFromNameOrUuid(std::string const& name) {
//    std::regex uuid_reg("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}");
//    if(std::regex_match(name,uuid_reg))
//        return mce::UUID::fromString(name);
    mce::UUID uuid = mce::UUID::fromString(name);
    if (!uuid) {
        auto suuid = PlayerInfo::getUUID(name);
        if (!suuid.empty())
        uuid = mce::UUID::fromString(suuid);
    }
    if (!uuid) {
        Global<Level>->forEachPlayer([&](Player& pl) {
            if (pl.getNameTag() == name) {
                auto suuid = pl.getUuid();
                if (!suuid.empty())
                    uuid = mce::UUID::fromString(suuid);
                return false;
            }
            return true;
            });
    }
    return uuid;
}
