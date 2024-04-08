#include "PlayerBagManager.h"

#include <memory>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"

namespace player_bag_manager {

static std::unique_ptr<PlayerBagManager> instance;

PlayerBagManager& PlayerBagManager::getInstance() { return *instance; }

bool PlayerBagManager::load() {
    getSelf().getLogger().info("Loading...");
    // Code for loading the plugin goes here.
    return true;
}

bool PlayerBagManager::enable() {
    getSelf().getLogger().info("Enabling...");
    // Code for enabling the plugin goes here.
    return true;
}

bool PlayerBagManager::disable() {
    getSelf().getLogger().info("Disabling...");
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace player_bag_manager

LL_REGISTER_PLUGIN(player_bag_manager::PlayerBagManager, player_bag_manager::instance);
