#include "PlayerBagManager.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/mod/RegisterHelper.h"
#include <memory>


namespace player_bag_manager {

ll::io::Logger& getLogger() { return PlayerBagManager::getInstance().getSelf().getLogger(); };

PlayerBagManager& PlayerBagManager::getInstance() {
    static PlayerBagManager instance;
    return instance;
}

bool PlayerBagManager::load() {
    getSelf().getLogger().debug("Loading...");
    // load i18n
    if (auto res = ll::i18n::getInstance().load(mSelf.getLangDir()); !res) {
        mSelf.getLogger().error("i18n load failed");
        res.error().log(mSelf.getLogger());
    }
    return true;
}

bool PlayerBagManager::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool PlayerBagManager::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace player_bag_manager

LL_REGISTER_MOD(player_bag_manager::PlayerBagManager, player_bag_manager::PlayerBagManager::getInstance());