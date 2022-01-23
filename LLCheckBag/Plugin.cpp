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
output.error("此子命令只允许由玩家执行");\
else


#define GetPlayerOrReturn()\
getPlayerFromOrigin(origin);\
if (!player)\
	return output.error("此子命令只允许由玩家执行");

#define CheckResultOutput(result, operation)\
if (result == CheckBagManager::Result::Success) \
	output.success(operation "成功");\
else {\
	output.error(operation "失败");\
	output.error("原因：" + CheckBagManager::getResultString(result));\
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
		if (!uuid){
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
					output.error("发送表单失败");
			}
			else {
				auto uuid = getTargetUuid();
				auto player = getPlayerFromOrigin(origin);
				auto result = manager.startCheckBag(player, uuid);
				CheckResultOutput(result, "开始检查玩家背包");
			}
			break;
		case LLCheckBagCommand::Operation::Rollback: {
			auto player = GetPlayerOrReturn();
			auto result = manager.restoreBagData(player);
			CheckResultOutput(result, "恢复玩家背包");
			break;
		}
		case LLCheckBagCommand::Operation::Overwrite: {
			auto player = GetPlayerOrReturn();
			auto result = manager.overwriteData(player);
			CheckResultOutput(result, "覆盖玩家数据");
			break;
		}
		case LLCheckBagCommand::Operation::Stop: {
			auto player = GetPlayerOrReturn();
			auto result = manager.stopCheckBag(player);
			CheckResultOutput(result, "停止查包");
			break; 
		}
		case LLCheckBagCommand::Operation::Remove:
			if (!mPlayer_isSet) {
				auto player = GetPlayerOrReturn();
				if (!FormHelper::openRemoveDataScreen(player))
					output.error("发送表单失败");
			}
			else {
				auto uuid = getTargetUuid();
				auto result = manager.removePlayerData(uuid);
				CheckResultOutput(result, "移除玩家数据");
			}
			break;
		case LLCheckBagCommand::Operation::Menu: {
			auto player = GetPlayerOrReturn();
			if (!FormHelper::openMenuScreen(player))
				output.error("发送表单失败");
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
					output.error("发送表单失败");
			}
			else {
				auto dataType = mDataType_isSet ? mDataType : NbtDataType::Snbt;
				auto result = manager.exportData(mPlayer, dataType);
				CheckResultOutput(result, "导出玩家数据");

			}
			break;
		}
		case LLCheckBagCommand::Operation::Import: {
			//return output.error("暂不支持导入功能");
			auto player = GetPlayerOrReturn();
			if (!FormHelper::openImportScreen(player))
				output.error("发送表单失败");
			break;
		}
		case LLCheckBagCommand::Operation::ExportAll: {
			auto dataType = mDataType_isSet ? mDataType : NbtDataType::Snbt;
			auto count = CheckBagMgr.exportAllData(dataType);
			if (count > 0)
				output.success(fmt::format("成功导出 {} 个玩家的数据", count));
			else
				output.error("没有可导出的数据或数据导出失败");
			break;
		}
		default:
			break;
		}
	}

	void execute(CommandOrigin const& origin, CommandOutput& output) const {
		Player* player = Command::getPlayerFromOrigin(origin);
		if (!Config::isOP(player->getRealName())) {
			output.error("抱歉，你没有使用这个指令的权限");
			return;
		}
		if (!mOperation_isSet) {
			auto player = GetPlayerOrReturn();
			if (!FormHelper::openCheckBagSmartScreen(player))
				output.error("发送表单失败");
			return;
		}else
			return checkBagCli(origin, output);
	}

public:

	static void setup(CommandRegistry* registry) {
		auto& manager = CheckBagMgr;
		registry->registerCommand("llcheckbag", "检查玩家背包", CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
		registry->registerAlias("llcheckbag", Config::CommandAlias);

		registry->addSoftEnum("LLCheckBag_PlayerList", manager.getPlayerList());

		registry->addEnum<Operation>("LLCheckBag_Action", {
			{"rollback",Operation::Rollback},
			{"overwrite",Operation::Overwrite},
			{"stop",Operation::Stop},
			{"menu",Operation::Menu},
			{"list",Operation::List},
			{"import",Operation::Import},
			//{"addop",Operation::AddOp},
			{"rb",Operation::Rollback},
			{"ow",Operation::Overwrite},
			{"s",Operation::Stop},
			{"m",Operation::Menu},
			{"l",Operation::List},
			{"i",Operation::Import},
			//{"ao",Operation::AddOp},
			});
		registry->addEnum<Operation>("LLCheckBag_ActionWithPlayer", {
			{"check",Operation::Check},
			{"remove",Operation::Remove},
			{"c",Operation::Check},
			{"rm",Operation::Remove},
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
	Config::initOp();
	
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