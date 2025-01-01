
#include "Config.h"
#include "ll/api/Config.h"
#include "ll/api/utils/ErrorUtils.h"
#include "PlayerBagManager.h"


namespace player_bag_manager::config {

bool addOperator(std::string xuid) {
    getConfig().operatorXuidList.push_back(xuid);
    auto res = isOperator(xuid);
    res      = res && saveConfigFile();
    return res;
}

bool isOperator(std::string xuid) {

    auto res = std::find(getConfig().operatorXuidList.begin(), getConfig().operatorXuidList.end(), xuid);
    return res != getConfig().operatorXuidList.end();
}

bool loadConfigFile() {
    auto& configPath = player_bag_manager::PlayerBagManager::getInstance().getSelf().getConfigDir();
    if (ll::config::loadConfig(getConfig(), configPath)) {
        getLogger().info("Config file load success!");
        return true;
    }
    getLogger().error("Config file load fail, will rewrite default config!");
            
    if (ll::config::saveConfig(getConfig(), configPath)) {
        getLogger().warn("Config file rewrite success!");
        return true;
    } else {
        getLogger().error("Config rewrite failed!");
    }
    getLogger().error("Config rewrite failed!");
    return false;
};

bool saveConfigFile() {
    if(ll::config::saveConfig(getConfig(), player_bag_manager::PlayerBagManager::getInstance().getSelf().getConfigDir())) {
        getLogger().info("Config file save success!");
        return true;
    }
    else {
        getLogger().error("Config file save fail!");
    }
    return false;
}
} // namespace player_bag_manager::config