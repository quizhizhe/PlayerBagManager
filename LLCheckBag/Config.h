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

namespace Config {
    static bool PacketMode = false;
    static bool MsaIdOnly = false;
    static std::string CommandAlias = "llcb";
    static std::string BackupDirectory = "plugins/LLCheckBag/Backup/";
    static std::string ExportDirectory = "plugins/LLCheckBag/Export/";
    static bool CheckLLFakePlayer = true;

    bool initConfig();
}