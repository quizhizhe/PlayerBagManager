#pragma once

#include "ll/api/io/Logger.h"
#include "ll/api/mod/NativeMod.h"


namespace player_bag_manager {

ll::io::Logger& getLogger();

class PlayerBagManager {

public:
    static PlayerBagManager& getInstance();

    PlayerBagManager() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace player_bag_manager
