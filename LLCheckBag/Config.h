#pragma once
#include "pch.h"

//#define PLUGIN_DEV_MODE
#include "DebugHelper.h"

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
#define PLUGIN_OP_PATH "plugins/LLCheckBag/op.json"
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
    ExportAll,
};

enum class PlayerCategory :int {
    All,
    Normal,
    FakePlayer,
    Unnamed,
};

template <typename T>
inline T fromString(std::string const& name);
std::string toString(NbtDataType type);
std::string toString(ScreenCategory type);
std::string toString(PlayerCategory type);
//template NbtDataType fromString<NbtDataType>(std::string const& name);
//template ScreenCategory fromString<ScreenCategory>(std::string const& name);
//template PlayerCategory fromString<PlayerCategory>(std::string const& name);

namespace Config {
    static nlohmann::json op;
    static bool PacketMode = false;
    static bool MsaIdOnly = false;
    static bool GuiWithCategory = true;
    static std::string CommandAlias = "llcb";
    static std::string BackupDirectory = "plugins/LLCheckBag/Backup/";
    static std::string ExportDirectory = "plugins/LLCheckBag/Export/";
    //static bool CheckLLFakePlayer = true;
    static NbtDataType BackupDataType = NbtDataType::Binary;
    static ScreenCategory DefaultScreen = ScreenCategory::Check;
    bool isOP(std::string xuid);

    bool initConfig();
    bool initOp();
}