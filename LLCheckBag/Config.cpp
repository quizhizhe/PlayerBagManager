#include "pch.h"
#include "Config.h"
#include <third-party/Nlohmann/json.hpp>
#include <filesystem>

#define CaseEnumValue(type, value)\
case type::value:\
    return #value
#define IfEnumValue(type, value)\
if (name == toLowerCase(#value))\
    return type::value;

#define SerializeVaule(var) json[#var] = Config::var
#define SerializeEnumVaule(var) json[#var] = toString(Config::var)

#define DeserializeVaule(var)\
if (json.find(#var) != json.end()){\
    Config::var = json.value(#var, Config::var);\
}\
else{\
    logger.info("Missing Config {}, use default value ", #var/*, Config::var*/);\
    needUpdate = true;\
}

#define DeserializeEnumVaule(var)\
if (json.find(#var) != json.end()){\
    auto svar = toString(Config::var);\
    svar = json.value(#var, svar); \
    Config::var = fromString<decltype(Config::var)>(svar);\
}\
else{\
    logger.warn("Missing Config {}, use default value {}", #var, toString(Config::var));\
    needUpdate = true;\
}

inline std::string toLowerCase(std::string const& name) {
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    return lname;
}

// =========== NbtDataType Converter ===========
inline std::string toString(NbtDataType type) {
    switch (type)
    {
        CaseEnumValue(NbtDataType, Snbt);
        CaseEnumValue(NbtDataType, Binary);
        CaseEnumValue(NbtDataType, Json);
    default:
        return "Binary";
    }
}

template<>
inline NbtDataType fromString(std::string const& value) {
    std::string name = toLowerCase(value);

    IfEnumValue(NbtDataType, Snbt);
    IfEnumValue(NbtDataType, Binary);
    IfEnumValue(NbtDataType, Json);

    return NbtDataType::Binary;
}

// =========== ScreenCategory Converter ===========
inline std::string toString(ScreenCategory type) {
    switch (type)
    {
        CaseEnumValue(ScreenCategory, Check);
        CaseEnumValue(ScreenCategory, Menu);
        CaseEnumValue(ScreenCategory, Import);
        CaseEnumValue(ScreenCategory, Export);
        CaseEnumValue(ScreenCategory, Delete);
        CaseEnumValue(ScreenCategory, ExportAll);
    default:
        return "Check";
        break;

    }
}

template<>
inline ScreenCategory fromString(std::string const& value) {
    std::string name = toLowerCase(value);

    IfEnumValue(ScreenCategory, Check);
    IfEnumValue(ScreenCategory, Menu);
    IfEnumValue(ScreenCategory, Import);
    IfEnumValue(ScreenCategory, Export);
    IfEnumValue(ScreenCategory, Delete);
    IfEnumValue(ScreenCategory, ExportAll);

    return ScreenCategory::Check;
}

// =========== PlayerCategory Converter ===========
std::string toString(PlayerCategory type) {
    switch (type)
    {
        CaseEnumValue(PlayerCategory, All);
        CaseEnumValue(PlayerCategory, Normal);
        CaseEnumValue(PlayerCategory, FakePlayer);
        CaseEnumValue(PlayerCategory, Unnamed);
    default:
        return "Normal";
        break;

    }
}

template<>
PlayerCategory fromString(std::string const& value) {
    std::string name = toLowerCase(value);

    IfEnumValue(PlayerCategory, All);
    IfEnumValue(PlayerCategory, Normal);
    IfEnumValue(PlayerCategory, FakePlayer);
    IfEnumValue(PlayerCategory, Unnamed);

    return PlayerCategory::Normal;
}

namespace Config {
    inline std::string serialize() {
        nlohmann::json json;

        //SerializeVaule(PacketMode);
        SerializeVaule(MsaIdOnly);
        SerializeVaule(CommandAlias);
        SerializeVaule(BackupDirectory);
        SerializeVaule(ExportDirectory);
        SerializeVaule(CustomOperator);
        SerializeVaule(OperatorXuidList);
        //SerializeVaule(CheckLLFakePlayer);
        SerializeEnumVaule(BackupDataType);
        SerializeEnumVaule(DefaultScreen);
        return json.dump(4);
    }

    inline bool deserialize(std::string jsonStr) {
        auto json = nlohmann::json::parse(jsonStr, nullptr, false, true);
        bool needUpdate = false;

        //DeserializeVaule(PacketMode);
        DeserializeVaule(MsaIdOnly);
        DeserializeVaule(CommandAlias);
        DeserializeVaule(BackupDirectory);
        DeserializeVaule(ExportDirectory);
        DeserializeVaule(CustomOperator);
        DeserializeVaule(OperatorXuidList);
        //DeserializeVaule(CheckLLFakePlayer);
        DeserializeEnumVaule(BackupDataType);
        DeserializeEnumVaule(DefaultScreen);

        if ((BackupDataType != NbtDataType::Binary) && (BackupDataType != NbtDataType::Snbt)) {
            needUpdate = true;
            BackupDataType = NbtDataType::Binary;
        }
        return !needUpdate;
    }

    bool addOperator(std::string xuid) {
        Config::OperatorXuidList.push_back(xuid);
        auto res = isOperator(xuid);
        res = res && saveConfig();
        return res;
    }

    bool isOperator(std::string xuid) {
        auto res = std::find(Config::OperatorXuidList.begin(), Config::OperatorXuidList.end(), xuid);
        return  res != Config::OperatorXuidList.end();
    }

    bool initConfig() {
        bool res = false;
        auto jsonStr = ReadAllFile(PLUGIN_CONFIG_PATH);
        if (jsonStr.has_value()) {
            try
            {
                res = deserialize(jsonStr.value());
            }
            catch (const std::exception&)
            {
                logger.error("Error in loading config file \"{}\", Use default config", PLUGIN_CONFIG_PATH);
                res = false;
            }
        }
        else {
            logger.info("Config File \"{}\" Not Found, Use Default Config", PLUGIN_CONFIG_PATH);
            std::filesystem::create_directories(std::filesystem::path(PLUGIN_CONFIG_PATH).remove_filename());
        }
        if (!res) {
            res = saveConfig();
        }
        std::filesystem::create_directories(Config::BackupDirectory);
        std::filesystem::create_directories(Config::ExportDirectory);
        return res;
    };

    bool saveConfig() {
        return WriteAllFile(PLUGIN_CONFIG_PATH, serialize(), false);
    }
}