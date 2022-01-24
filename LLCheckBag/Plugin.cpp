#include "pch.h"
#include "Config.h"
#include "CheckBagManager.h"
#include <PlayerInfoAPI.h>
#include <FormUI.h>
#include "FormHelper.h"

using namespace RegisterCommandHelper;

Logger logger("LLCheckBag");

#define IfExecuteByPlayer()\
auto player = getPlayerFromOrigin(origin);\
if (!player)\
output.error(tr("command.only_player"));\
else


#define GetPlayerOrReturn()\
getPlayerFromOrigin(origin);\
if (!player)\
	return output.error(tr("command.only_player"));

#define CheckResultOutput(result, operation)\
if (result == CheckBagManager::Result::Success) \
	output.success(tr("operation.success",operation));\
else {\
	output.error(tr("operation.failed", operation, CheckBagManager::getResultString(result)));\
}

class LLCheckBagCommand : public Command {

    enum class Operation {
        Check,
        Rollback,
        Overwrite,
        Stop,
        Remove,
        List,
        Import,
        Export,
        Menu,
        ExportAll,
        AddOp,
    } mOperation;
    std::string mPlayer;
    NbtDataType mDataType = NbtDataType::Snbt;
    bool mOperation_isSet = false;
    bool mPlayer_isSet = false;
    bool mDataType_isSet = false;

    mce::UUID getTargetUuid() const {
        auto uuid = mce::UUID::fromString(mPlayer);
        if (!uuid) {
            auto uuidStr = PlayerInfo::getUUID(mPlayer);
            uuid = mce::UUID::fromString(uuidStr);
        }
        return uuid;
    }

    void checkBagCli(CommandOrigin const& origin, CommandOutput& output) const {
        auto& manager = CheckBagMgr;
        switch (mOperation)
        {
        case LLCheckBagCommand::Operation::Check:
            if (!mPlayer_isSet) {
                auto player = GetPlayerOrReturn();
                if (!FormHelper::openCheckBagSmartScreen(player))
                    output.error(tr("screen.send.error"));
            }
            else {
                auto uuid = getTargetUuid();
                auto player = getPlayerFromOrigin(origin);
                auto result = manager.startCheckBag(player, uuid);
                CheckResultOutput(result, tr("operation.start_check"));
            }
            break;
        case LLCheckBagCommand::Operation::Rollback: {
            auto player = GetPlayerOrReturn();
            auto result = manager.restoreBagData(player);
            CheckResultOutput(result, tr("operation.restore"));
            break;
        }
        case LLCheckBagCommand::Operation::Overwrite: {
            auto player = GetPlayerOrReturn();
            auto result = manager.overwriteData(player);
            CheckResultOutput(result, tr("operation.overwrite"));
            break;
        }
        case LLCheckBagCommand::Operation::Stop: {
            auto player = GetPlayerOrReturn();
            auto result = manager.stopCheckBag(player);
            CheckResultOutput(result, tr("operation.stop"));
            break;
        }
        case LLCheckBagCommand::Operation::Remove:
            if (!mPlayer_isSet) {
                auto player = GetPlayerOrReturn();
                if (!FormHelper::openRemoveDataScreen(player))
                    output.error(tr("screen.send.error"));
            }
            else {
                auto uuid = getTargetUuid();
                auto result = manager.removePlayerData(uuid);
                CheckResultOutput(result, tr("operation.remove"));
            }
            break;
        case LLCheckBagCommand::Operation::Menu: {
            auto player = GetPlayerOrReturn();
            if (!FormHelper::openMenuScreen(player))
                output.error(tr("screen.send.error"));
            break;
        }
        case LLCheckBagCommand::Operation::List: {
            size_t count = 0;
            for (auto& name : manager.getPlayerList()) {
                output.success(name);
            }
            break;
        }
        case LLCheckBagCommand::Operation::Export: {
            if (!mPlayer_isSet) {
                auto player = GetPlayerOrReturn();
                if (!FormHelper::openExportScreen(player))
                    output.error(tr("screen.send.error"));
            }
            else {
                auto dataType = mDataType_isSet ? mDataType : NbtDataType::Snbt;
                auto result = manager.exportData(mPlayer, dataType);
                CheckResultOutput(result, tr("operation.export"));

            }
            break;
        }
        case LLCheckBagCommand::Operation::Import: {
            //return output.error("暂不支持导入功能");
            auto player = GetPlayerOrReturn();
            if (!FormHelper::openImportScreen(player))
                output.error(tr("screen.send.error"));
            break;
        }
        case LLCheckBagCommand::Operation::ExportAll: {
            auto dataType = mDataType_isSet ? mDataType : NbtDataType::Snbt;
            auto count = CheckBagMgr.exportAllData(dataType);
            if (count > 0)
                output.success(tr("operation.export.success", count));
            else
                output.error(tr("operation.export.no_target"));
            break;
        }
        default:
            break;
        }
    }

    void addOp(CommandOrigin const& origin, CommandOutput& output) const {
        xuid_t xuid = "";
        for (auto& player : Level::getAllPlayers()) {
            if (player->getRealName() == mPlayer) {
                xuid = player->getXuid();
            }
        }
        if (!xuid.empty()) {
            if (Config::isOperator(xuid))
                return output.error(tr("operation.op.failed.already_in", mPlayer));
            Config::addOperator(xuid);
            return output.success(tr("operation.op.success", mPlayer));
        }
        return output.error(tr("operation.op.failed.not_found", mPlayer));
    }

    void execute(CommandOrigin const& origin, CommandOutput& output) const {
        if ((OriginType)origin.getOriginType() == OriginType::DedicatedServer
            && mOperation_isSet && mPlayer_isSet && mOperation == Operation::AddOp) {
            return addOp(origin, output);
        }
        if ((OriginType)origin.getOriginType() != OriginType::DedicatedServer) {
            Player* player = Command::getPlayerFromOrigin(origin);
            /// 防止 execute 绕过权限限制？
            if (!player || !Config::isOperator(player->getXuid())) {
                return output.error(tr("command.no_permission"));
            }
        }
        if (!mOperation_isSet) {
            ASSERT(!mPlayer_isSet);
            auto player = GetPlayerOrReturn();
            bool res = false;
            switch (Config::DefaultScreen)
            {
            case ScreenCategory::Check:
                res = FormHelper::openCheckBagSmartScreen(player);
                break;
            case ScreenCategory::Menu:
                res = FormHelper::openMenuScreen(player);
                break;
            case ScreenCategory::Import:
                res = FormHelper::openImportScreen(player);
                break;
            case ScreenCategory::Export:
                res = FormHelper::openExportScreen(player);
                break;
            case ScreenCategory::Delete:
                res = FormHelper::openRemoveDataScreen(player);
                break;
            case ScreenCategory::ExportAll:
                res = FormHelper::openExportAllScreen(player);
                break;
            default:
                break;
            }
            if (!res)
                output.error(tr("screen.send.error"));
            return;
        }
        else
            return checkBagCli(origin, output);
    }

public:

    static void setup(CommandRegistry* registry) {
        auto& manager = CheckBagMgr;
        registry->registerCommand("llcheckbag", tr("command.description").c_str(), CommandPermissionLevel::Any, {(CommandFlagValue)0}, {(CommandFlagValue)0x80});
        registry->registerAlias("llcheckbag", Config::CommandAlias);

        registry->addSoftEnum("LLCheckBag_PlayerList", manager.getPlayerList());

        registry->addEnum<Operation>("LLCheckBag_Action", {
            {"rollback",Operation::Rollback},
            {"overwrite",Operation::Overwrite},
            {"stop",Operation::Stop},
            {"menu",Operation::Menu},
            {"list",Operation::List},
            {"import",Operation::Import},
            {"rb",Operation::Rollback},
            {"ow",Operation::Overwrite},
            {"s",Operation::Stop},
            {"m",Operation::Menu},
            {"l",Operation::List},
            {"i",Operation::Import},
            });
        registry->addEnum<Operation>("LLCheckBag_ActionWithPlayer", {
            {"check",Operation::Check},
            {"remove",Operation::Remove},
            {"c",Operation::Check},
            {"rm",Operation::Remove},
            {"op",Operation::AddOp},
            });
        registry->addEnum<Operation>("LLCheckBag_ActionExport", {
            {"export",Operation::Export},
            {"e",Operation::Export},
            });
        registry->addEnum<Operation>("LLCheckBag_ActionExportAll", {
            {"exportall",Operation::ExportAll},
            });
        registry->addEnum<NbtDataType>("LLCheckBag_ExportType", {
            {"snbt",NbtDataType::Snbt},
            {"binary",NbtDataType::Binary},
            {"json",NbtDataType::Json},
            {"s",NbtDataType::Snbt},
            {"b",NbtDataType::Binary},
            {"bin",NbtDataType::Binary},
            {"j",NbtDataType::Json},
            });

        auto action = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_Action", &LLCheckBagCommand::mOperation_isSet);
        auto actionWithPlayer = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_ActionWithPlayer", &LLCheckBagCommand::mOperation_isSet);
        auto actionExport = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_ActionExport", &LLCheckBagCommand::mOperation_isSet);
        auto actionExportAll = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_ActionExportAll", &LLCheckBagCommand::mOperation_isSet);
        auto dataTypeParam = makeOptional<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mDataType, "dataType", "LLCheckBag_ExportType", &LLCheckBagCommand::mDataType_isSet);

        action.addOptions((CommandParameterOption)1);
        actionWithPlayer.addOptions((CommandParameterOption)1);
        actionExport.addOptions((CommandParameterOption)1);
        actionExportAll.addOptions((CommandParameterOption)1);
        dataTypeParam.addOptions((CommandParameterOption)1);

        auto playerParam = makeOptional<CommandParameterDataType::SOFT_ENUM>(&LLCheckBagCommand::mPlayer, "player", "LLCheckBag_PlayerList", &LLCheckBagCommand::mPlayer_isSet);

        registry->registerOverload<LLCheckBagCommand>("llcheckbag");
        registry->registerOverload<LLCheckBagCommand>("llcheckbag", action);
        registry->registerOverload<LLCheckBagCommand>("llcheckbag", actionWithPlayer, playerParam);
        registry->registerOverload<LLCheckBagCommand>("llcheckbag", actionExport, playerParam, dataTypeParam);
        registry->registerOverload<LLCheckBagCommand>("llcheckbag", actionExportAll, dataTypeParam);
    }
};

void PluginInit()
{
    logger.setFile(PLUGIN_LOG_PATH);
    Config::initConfig();

    Event::RegCmdEvent::subscribe([](Event::RegCmdEvent ev) { // Register commands Event
        LLCheckBagCommand::setup(ev.mCommandRegistry);
        return true;
        });
    Event::PlayerJoinEvent::subscribe([](Event::PlayerJoinEvent ev) {
        CheckBagMgr.afterPlayerJoin((ServerPlayer*)ev.mPlayer);
        return true;
        });
    logger.info("LLCheckBag Loaded, version: {}", PLUGIN_VERSION_STRING);
}