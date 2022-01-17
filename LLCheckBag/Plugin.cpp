#include "pch.h"
#include "Config.h"
#include "CheckBagManager.h"
#include <PlayerInfoAPI.h>

using namespace RegisterCommandHelper;

Logger logger("LLCheckBag");


#define IfExecuteByPlayer()\
auto player = getPlayerFromOrigin(origin);\
if (!player)\
output.error("此子命令只允许由玩家执行");\
else

class LLCheckBagCommand : public Command {

	enum Operation {
		Check,
		Rollback,
		Overwrite,
		Stop,
		Remove,
		GUI, List, Export,
	} mOperation;
	std::string mPlayer;
	CheckBagManager::DataType mDataType = CheckBagManager::DataType::Snbt;
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
#define GetPlayerOrReturn()\
getPlayerFromOrigin(origin);\
if (!player)\
return output.error("此子命令只允许由玩家执行");

	void removePlayerGui(CommandOrigin const& origin, CommandOutput& output) const {
		auto player = GetPlayerOrReturn();

	}

	void checkBagGui(CommandOrigin const& origin, CommandOutput& output) const {
		auto player = GetPlayerOrReturn();

	}

	void exportGui(CommandOrigin const& origin, CommandOutput& output) const {
		auto player = GetPlayerOrReturn();

	}


	void checkBagCli(CommandOrigin const& origin, CommandOutput& output) const {
		auto& manager = CheckBagManager::getManager();
		switch (mOperation)
		{
		case LLCheckBagCommand::Check:
			if (!mPlayer_isSet)
				checkBagGui(origin, output);
			else {
				auto uuid = getTargetUuid();
				auto player = getPlayerFromOrigin(origin);
					manager.startCheckBag(player, uuid);
			}
			break;
		case LLCheckBagCommand::Rollback: {
			auto player = GetPlayerOrReturn();
			manager.restoreBagData(player);
		}
			break;
		case LLCheckBagCommand::Overwrite: {
			auto player = GetPlayerOrReturn();
			manager.overwriteData(player);
			break;
		}
		case LLCheckBagCommand::Stop: {
			auto player = GetPlayerOrReturn();
			manager.stopCheckBag(player);
			break; 
		}
		case LLCheckBagCommand::Remove:
			if (!mPlayer_isSet)
				removePlayerGui(origin, output);
			else {
				auto uuid = getTargetUuid();
				manager.removePlayerData(uuid);
			}
			break;
		case LLCheckBagCommand::GUI:
			checkBagGui(origin, output);
			break;
		case LLCheckBagCommand::List: {
			std::string list = "";
			size_t count = 0;
			for (auto& name : manager.getPlayerList()) {
				if (count++ != 0)
					list.append("\n");
				list.append(name);
			}
			output.success(list);
			break;
		}
		case LLCheckBagCommand::Export: {
			if (!mPlayer_isSet)
				exportGui(origin, output);
			else {
				auto dataType = mDataType_isSet ? mDataType : CheckBagManager::DataType::Snbt;
				manager.exportData(mPlayer, dataType);
			}
			break;
		}
		default:
			break;
		}
	}
	void execute(CommandOrigin const& origin, CommandOutput& output) const {
		Player* player = Command::getPlayerFromOrigin(origin);
		if (!mOperation_isSet)
			return checkBagGui(origin, output);
		return checkBagCli(origin, output);
	}

public:

	static void setup(CommandRegistry* registry) {
		auto& manager = CheckBagManager::getManager();
		registry->registerCommand("llcheckbag", "检查玩家背包", CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
		registry->registerAlias("llcheckbag", Config::CommandAlias);

		registry->addSoftEnum("LLCheckBag_PlayerList", manager.getPlayerList());

		registry->addEnum<Operation>("LLCheckBag_Action", {
			{"rollback",Operation::Rollback},
			{"overwrite",Operation::Overwrite},
			{"stop",Operation::Stop},
			{"gui",Operation::GUI},
			{"list",Operation::List},
			});
		registry->addEnum<Operation>("LLCheckBag_ActionWithPlayer", {
			{"check",Operation::Check},
			{"remove",Operation::Remove},
			});
		registry->addEnum<Operation>("LLCheckBag_ActionExport", {
			{"export",Operation::Export},
			});

		registry->addEnum<CheckBagManager::DataType>("LLCheckBag_ExportType", {
			{"snbt",CheckBagManager::DataType::Snbt},
			{"binary",CheckBagManager::DataType::Binary},
			{"json",CheckBagManager::DataType::Json},
			});

		auto action = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_Action", &LLCheckBagCommand::mOperation_isSet);
		auto actionWithPlayer = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_ActionWithPlayer", &LLCheckBagCommand::mOperation_isSet);
		auto actionExport = makeMandatory<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mOperation, "action", "LLCheckBag_ActionExport", &LLCheckBagCommand::mOperation_isSet);

		action.addOptions((CommandParameterOption)1);
		actionWithPlayer.addOptions((CommandParameterOption)1);
		actionExport.addOptions((CommandParameterOption)1);

		auto playerParam = makeOptional<CommandParameterDataType::SOFT_ENUM>(&LLCheckBagCommand::mPlayer, "player", "LLCheckBag_PlayerList", &LLCheckBagCommand::mPlayer_isSet);
		auto dataTypeParam = makeOptional<CommandParameterDataType::ENUM>(&LLCheckBagCommand::mDataType, "dataType", "LLCheckBag_ExportType", &LLCheckBagCommand::mDataType_isSet);


		registry->registerOverload<LLCheckBagCommand>("llcheckbag");
		registry->registerOverload<LLCheckBagCommand>("llcheckbag", action);
		registry->registerOverload<LLCheckBagCommand>("llcheckbag", actionWithPlayer, playerParam);
		registry->registerOverload<LLCheckBagCommand>("llcheckbag", actionExport, playerParam, dataTypeParam);
	}
};

void PluginInit()
{
	auto&& version = LL::Version(
		PLUGIN_VERSION_MAJOR,
		PLUGIN_VERSION_MINOR,
		PLUGIN_VERSION_REVISION,
		PLUGIN_VERSION_IS_BETA ? LL::Version::Beta : LL::Version::Release
	);
	Config::initConfig();
	LL::registerPlugin("LLCheckBag", "Introduction", version);

	Event::RegCmdEvent::subscribe([](Event::RegCmdEvent ev) { // Register commands Event
		LLCheckBagCommand::setup(ev.mCommandRegistry);
		return true;
		});
	logger.info("LLCheckBag Loaded, version: {}", PLUGIN_VERSION_STRING);
}