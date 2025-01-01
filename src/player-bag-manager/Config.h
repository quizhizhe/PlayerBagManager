#pragma once
#include "Version.h"
#include <string>
#include <vector>

// Version
#define PLUGIN_VERSION_IS_BETA PLUGIN_VERSION_STATUS != PLUGIN_VERSION_RELEASE

#if PLUGIN_VERSION_IS_BETA
#define PLUGIN_VERSION_STRING TO_VERSION_STRING(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION beta)
#else
#define PLUGIN_VERSION_STRING TO_VERSION_STRING(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION)
#endif // PLUGIN_VERSION_IS_BETA


#define PLUGIN_CONFIG_PATH  "plugins/LLCheckBag/config.json"
#define PLUGIN_LOG_PATH     "logs/LLCheckBag.log"
#define PLUGIN_LANGUAGE_DIR "plugins/LLCheckBag/Language/"

namespace player_bag_manager {

enum class NbtDataType : int {
    Snbt,
    Binary,
    Json,
    Unknown,
};

enum class ScreenCategory : int {
    Check,
    Menu,
    Import,
    Export,
    Delete,
    ExportAll,
};

enum class PlayerCategory : int {
    All,
    Normal,
    FakePlayer,
    Unnamed,
};

namespace config {

struct Config {

    int         version         = 2;
    bool        msaIdOnly       = false;
    bool        packetMode      = false;
    bool        guiWithCategory = true;
    bool        formattedSNBT   = true;
    std::string language        = "system";
    std::string commandAlias    = "llcb";
    std::string backupDirectory = "plugins/LLCheckBag/Backup/";
    std::string exportDirectory = "plugins/LLCheckBag/Export/";
    // static bool CheckLLFakePlayer = true;
    NbtDataType              backupDataType   = NbtDataType::Binary;
    ScreenCategory           defaultScreen    = ScreenCategory::Check;
    bool                     customOperator   = true;
    std::vector<std::string> operatorXuidList = {};
};

static Config& getConfig() {
    static Config instance;
    return instance;
}

bool addOperator(std::string xuid);
bool isOperator(std::string xuid);
bool loadConfigFile();
bool saveConfigFile();

} // namespace config

} // namespace player_bag_manager