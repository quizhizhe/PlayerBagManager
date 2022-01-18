#include "pch.h"
#include "Config.h"
#include <third-party/Nlohmann/json.hpp>
#include <filesystem>

#define SerializeVaule(var) json[#var] = Config::var
#define DeserializeVaule(var)\
if (json.find(#var) != json.end())\
    Config::var = json.value(#var, Config::var);\
else{\
    logger.warn("Missing Config {}, use default value {}", #var, Config::var);\
    needUpdate = true;\
}

namespace Config {
    inline std::string serialize() {
        nlohmann::json json;

        SerializeVaule(PacketMode);
        SerializeVaule(MsaIdOnly);
        SerializeVaule(CommandAlias);
        SerializeVaule(BackupDirectory);
        SerializeVaule(ExportDirectory);
        //SerializeVaule(CheckLLFakePlayer);
        //SerializeVaule(BackupDataType);

        return json.dump(4);
    }
    inline bool deserialize(std::string jsonStr) {
        auto json = nlohmann::json::parse(jsonStr, nullptr, true, true);
        bool needUpdate = false;

        DeserializeVaule(PacketMode);
        DeserializeVaule(MsaIdOnly);
        DeserializeVaule(CommandAlias);
        DeserializeVaule(BackupDirectory);
        DeserializeVaule(ExportDirectory);
        //DeserializeVaule(CheckLLFakePlayer);
        //DeserializeVaule(BackupDataType);

        if (needUpdate) {
            WriteAllFile(PLUGIN_CONFIG_PATH, serialize(), false);
        }
        return true;
    }
    bool initConfig() {
        bool res = false;
        auto jsonStr = ReadAllFile(PLUGIN_CONFIG_PATH);
        if (jsonStr.has_value()) {
            res = deserialize(jsonStr.value());
        }
        if(!res) {
            logger.warn("Config File \"{}\" Not Found, Use Default Config", PLUGIN_CONFIG_PATH);
            std::filesystem::create_directories(std::filesystem::path(PLUGIN_CONFIG_PATH).remove_filename());
            res = WriteAllFile(PLUGIN_CONFIG_PATH, serialize(), false);
        }
        std::filesystem::create_directories(Config::BackupDirectory);
        std::filesystem::create_directories(Config::ExportDirectory);
        return res;
    };
}