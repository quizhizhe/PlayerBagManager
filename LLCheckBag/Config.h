#pragma once
#include "pch.h"

#include "DebugHelper.h"

// Version
#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 2
#define PLUGIN_VERSION_REVISION 3
#ifdef PLUGIN_DEV_MODE
#define PLUGIN_VERSION_IS_BETA true
#else
#define PLUGIN_VERSION_IS_BETA false
#endif // PLUGIN_DEV_MODE


#define STR1(R) #R
#define STR2(R) STR1(R)

#if PLUGIN_VERSION_IS_BETA
#define PLUGIN_VERSION_STRING STR2(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION beta)
#else
#define PLUGIN_VERSION_STRING STR2(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION)
#endif // PLUGIN_VERSION_IS_BETA

extern Logger logger;

#define PLUGIN_CONFIG_PATH "plugins/LLCheckBag/config.json"
#define PLUGIN_LOG_PATH "logs/LLCheckBag.log"
#define PLUGIN_LANGUAGE_DIR "plugins/LLCheckBag/Language/"

enum class NbtDataType :int {
    Snbt,
    Binary,
    Json,
    Unknown,
};

enum class ScreenCategory :int {
    Check,
    Menu,
    Import,
    Export,
    Delete,
    ExportAll,
};

enum class PlayerCategory :int {
    All,
    Normal,
    FakePlayer,
    Unnamed,
};

namespace Config {
    static int ConfigVersion = 1;
    static bool PacketMode = false;
    static bool MsaIdOnly = false;
    static bool GuiWithCategory = true;
    static bool FormattedSNBT = true;
    static std::string Language = "zh_CN";
    static std::string CommandAlias = "llcb";
    static std::string BackupDirectory = "plugins/LLCheckBag/Backup/";
    static std::string ExportDirectory = "plugins/LLCheckBag/Export/";
    //static bool CheckLLFakePlayer = true;
    static NbtDataType BackupDataType = NbtDataType::Binary;
    static ScreenCategory DefaultScreen = ScreenCategory::Check;
    static bool CustomOperator = true;
    static std::vector<std::string> OperatorXuidList = {};

    bool addOperator(std::string xuid);
    bool isOperator(std::string xuid);
    bool initConfig();
    bool saveConfig();
}