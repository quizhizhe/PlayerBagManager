#include "pch.h"
#include "Config.h"
#include <third-party/Nlohmann/json.hpp>
#include <filesystem>
#include <TranslationAPI.h>
#include <PlayerInfoAPI.h>

#define SerializeVaule(var) json[#var] = Config::var
#define SerializeEnumVaule(var) json[#var] = magic_enum::enum_name(Config::var)

#define DeserializeVaule(var)\
if (json.find(#var) != json.end()){\
    Config::var = json.value(#var, Config::var);\
}\
else{\
    logger.info("Missing config {}, use default value ", #var/*, Config::var*/);\
    needUpdate = true;\
}

#define DeserializeEnumVaule(var)\
if (json.find(#var) != json.end()){\
    auto svar = magic_enum::enum_name(Config::var);\
    svar = json.value(#var, svar); \
    auto enumValue = magic_enum::enum_cast<decltype(Config::var)>(svar);\
    if (enumValue.has_value())\
        Config::var = enumValue.value();\
    else{\
        logger.warn("Unsupported config value {}, use default value {}", svar, magic_enum::enum_name(Config::var));\
        needUpdate = true;\
    }\
}\
else{\
    logger.warn("Missing config {}, use default value {}", #var, magic_enum::enum_name(Config::var));\
    needUpdate = true;\
}

namespace Config {
    inline std::string serialize() {
        nlohmann::json json;

        SerializeVaule(ConfigVersion);
        //SerializeVaule(PacketMode);
        SerializeVaule(MsaIdOnly);
        SerializeVaule(Language);
        SerializeVaule(CommandAlias);
        SerializeVaule(FormattedSNBT);
        SerializeVaule(BackupDirectory);
        SerializeVaule(ExportDirectory);
        SerializeVaule(CustomOperator);
        SerializeVaule(OperatorXuidList);
        //SerializeVaule(CheckLLFakePlayer);
        SerializeEnumVaule(BackupDataType);
        SerializeEnumVaule(DefaultScreen);
        return json.dump(4);
    }

    // return needUpdate
    inline bool checkConfigCompatible(nlohmann::json& json) {
        int cfgver = 0;
        cfgver = json.value("ConfigVersion", 0);
        if (cfgver == Config::ConfigVersion)
            return false;
        logger.info("update config from ConfigVersion {}", cfgver);
        switch (cfgver)
        {
        case 0: {
            auto opsPath = ReadAllFile("plugins/LLCheckBag/op.json");
            if (opsPath.has_value()) {
                std::vector<std::string> opNames;
                std::vector<std::string> opXuids;
                auto opjson = nlohmann::json::parse(opsPath.value(), nullptr, false, true);
                opNames = opjson.value("name", opNames);
                for (auto& opName : opNames) {
                    auto xuid = PlayerInfo::getXuid(opName);
                    if (!xuid.empty())
                        opXuids.push_back(xuid);
                }
                json["OperatorXuidList"] = opXuids;
                std::filesystem::remove("plugins/LLCheckBag/op.json");
            }
        }
        default:
            break;
        }
        return false;
    }

    inline bool deserialize(std::string jsonStr) {
        auto json = nlohmann::json::parse(jsonStr, nullptr, false, true);
        bool needUpdate = false;

        needUpdate = checkConfigCompatible(json);

        //DeserializeVaule(PacketMode);
        DeserializeVaule(MsaIdOnly);
        DeserializeVaule(Language);
        DeserializeVaule(CommandAlias);
        DeserializeVaule(FormattedSNBT);
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
    bool loadLanguage(std::string language) {
        auto languageFile = std::filesystem::path(PLUGIN_LANGUAGE_DIR);
        languageFile.append(Config::Language + ".json");
        if (!std::filesystem::exists(languageFile)) {
            languageFile = std::filesystem::path(PLUGIN_LANGUAGE_DIR);
            languageFile.append("zh_CN.json");
            if (!std::filesystem::exists(languageFile)) {
                return false;
            }
        }
        return Translation::load(languageFile.string());
        
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
        if (!loadLanguage(Config::Language)) {
            logger.error("Error loading language file!");
            throw("Error loading language file!");
        }
        return res;
    };

    bool saveConfig() {
        return WriteAllFile(PLUGIN_CONFIG_PATH, serialize(), false);
    }
}