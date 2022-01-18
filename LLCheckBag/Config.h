#pragma once
#include "pch.h"


// Version
#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 1
#define PLUGIN_VERSION_REVISION 0
#define PLUGIN_VERSION_IS_BETA true

#define STR1(R) #R
#define STR2(R) STR1(R)

#if PLUGIN_VERSION_IS_BETA
#define PLUGIN_VERSION_STRING STR2(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION beta)
#else
#define PLUGIN_VERSION_STRING STR2(PLUGIN_VERSION_MAJOR.PLUGIN_VERSION_MINOR.PLUGIN_VERSION_REVISION)
#endif // PLUGIN_VERSION_IS_BETA

extern Logger logger;

#define PLUGIN_CONFIG_PATH "plugins/LLCheckBag/config.json"
#define PLUGIN_LOG_PATH "plugins/logs/LLCheckBag.log"

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
};

inline std::string toString(NbtDataType type);
inline std::string toString(ScreenCategory type);
template <typename T>
inline T fromString(std::string const& name);

namespace Config {
    static bool PacketMode = false;
    static bool MsaIdOnly = false;
    static std::string CommandAlias = "llcb";
    static std::string BackupDirectory = "plugins/LLCheckBag/Backup/";
    static std::string ExportDirectory = "plugins/LLCheckBag/Export/";
    //static bool CheckLLFakePlayer = true;
    static NbtDataType BackupDataType = NbtDataType::Binary;
    static ScreenCategory DefaultScreen = ScreenCategory::Check;

    bool initConfig();
}