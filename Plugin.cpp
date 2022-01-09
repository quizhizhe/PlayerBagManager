#include "pch.h"
#include "DataHeleper.h"
#include "JsonHelper.h"
#include "SymHelp.h"

Logger logger("LLCheckBag");

bool onPlayerLogin(Event::PlayerJoinEvent ev) {
	std::cout << 1 << std::endl;
	Player* player = ev.mPlayer;
	std::string uuid = player->getUuid();
	std::cout << 2 << std::endl;
	std::string ouuid = findUuid(uuid);
	std::cout << 3 << std::endl;
	if (ouuid != uuid) {
		UuidStruct newPlayer;
		newPlayer.name = player->getRealName();
		newPlayer.xuid = player->getXuid();
		newPlayer.serverId = getServerId(player);
		if (tryToSaveUuid(uuid, newPlayer)) std::cout << "New Player Add!" << std::endl;
		else std::cout << "Error save player data" << std::endl;
	}
	std::cout << "Error save player data1" << std::endl;
	return true;
}


class LLCheckBagCommand : public Command {

public:
	void execute(CommandOrigin const& ori, CommandOutput& outp) const {
		Player* player = Command::getPlayerFromOrigin(ori);
		auto TagName = "_IS_CHECK_BAG_-"+player->getRealName();
		std::string	isCheck = PluginOwnData::get<std::string>(TagName);
		std::cout <<"[LLTest]" << isCheck << std::endl;
		if (!isCheck.empty()) {
			vector<string> btnList = {"结束查包，恢复自己的背包到原状"};
			vector<string> image(1);
			Player* target = Level::getPlayer(isCheck);
			if (target) {
				btnList.push_back("自己当前的背包 => 被查包玩家的背包");
				btnList.push_back("自己当前的背包 <= 被查包玩家的背包");
				image.push_back("");
				image.push_back("");
			}
			std::string content = "你正在检查" + isCheck + "玩家的背包。\n请选择一项操作：";
			player->sendSimpleFormPacket("LL Check Bag", content, btnList, image, [player,target,TagName](int id) {
				//std::cout << id << std::endl;
				switch (id) {
						
				case 0: {
					if (recoveBagData(player)) {
					PluginOwnData::remove<std::string>(TagName);
					}
					break;
				}
				case 1: {
					auto mp = player->getNbt();
					target->setNbt(&*mp);
					break;
				}
				case 2: {
					auto tp = target->getNbt();
					player->setNbt(&*tp);
					break;
				}
				default:
					break;
				}
				});
		}
		else {
			//vector<UuidStruct> dataList = getAllUuidData();//从文件获取所有uuid信息
			vector<string> btnList1 = getPlayerButton();
			std::vector<string> image1(btnList1.size());
			player->sendSimpleFormPacket("LL Check Bag", "Please select Player to check", btnList1, image1, [player, btnList1,TagName](int id) {
				if (id != NULL) {
					std::string name = btnList1[id];
					auto plSel = Level::getPlayer(name);
					std::unique_ptr<CompoundTag> plData;
					if (!plSel) {
						//离线玩家的处理
						auto suuid = findUuidByName(name);//从储存的数据查找uuid
						if (suuid.empty()) {
							plData = loadPlayer(name);
						}
						else {
							plData = loadPlayer(suuid);
						}
					}
					else {
						plData = loadPlayer(plSel);
					}
					saveBag(player);
					PluginOwnData::set<std::string>(TagName, name);
					copyBagData(player, std::move(plData));
					player->sendTextPacket("Bag is ready, you can checking");
				}
				});
		}
		
	}
	static void setup(CommandRegistry* registry) {
		registry->registerCommand("llcb", "Test llcb", CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
		registry->registerOverload<LLCheckBagCommand>("llcb");
	}
};

void PluginInit()
{
	LL::registerPlugin("LLCheckBag", "Introduction", LL::Version(1, 0, 0));
	loadUuiddb();
	Event::PlayerJoinEvent::subscribe(onPlayerLogin);
	Event::RegCmdEvent::subscribe([](Event::RegCmdEvent ev) { // Register commands Event
		LLCheckBagCommand::setup(ev.mCommandRegistry);
		return true;
		});
}