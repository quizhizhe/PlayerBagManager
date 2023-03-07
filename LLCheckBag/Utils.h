#pragma once
#include <llapi/Global.h>
#include <llapi/PlayerInfoAPI.h>
#include <llapi/mc/Level.hpp>
#include <llapi/mc/Player.hpp>

inline mce::UUID UuidFromNameOrUuid(std::string const& name) {
    auto uuid = mce::UUID::fromString(name);
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
