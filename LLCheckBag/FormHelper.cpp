#include "pch.h"
#include <map>
#include "FormHelper.h"
#include <MC/Packet.hpp>

namespace FormHelper {
    bool sendPlayerListForm(
        Player* player,
        std::string const& title,
        std::string const& content,
        std::function<void(Player* player, mce::UUID const& uuid)>&& callback,
        PlayerCategory category
    ) {
        Form::SimpleForm form(title, content); 
        TestFuncTime(CheckBagMgr.getPlayerList, category); // <0.5ms
        auto playerList = CheckBagMgr.getPlayerList(category);
        for (auto& name : playerList) {
            form.append(Form::Button(name));
        }
        return form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
            if (index < 0)
                return;
            auto& target = playerList[index];
            auto uuid = mce::UUID::fromString(target);
            if (!uuid) {
                auto suuid = PlayerInfo::getUUID(target);
                uuid = mce::UUID::fromString(suuid);
            }
            callback(player, uuid);
        });
    }

    bool sendPlayerCategoryForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, PlayerCategory category)>&& callback) {
        static std::vector<std::string> PlayerCategorys{
            toString(PlayerCategory::All),
            toString(PlayerCategory::Normal),
            toString(PlayerCategory::FakePlayer),
            toString(PlayerCategory::Unnamed),
        };

        Form::SimpleForm form(title, "请选择分类");

        for (auto& btn : PlayerCategorys) {
            form.append(Form::Button{ btn });
        }

        return form.sendTo((ServerPlayer*)player, [title, content, player, callback](int index) {
            if (index < 0)
                return;
            auto category = fromString<PlayerCategory>(PlayerCategorys[index]);
            callback(player, category);
            });
    }

    bool sendPlayerListWithCategoryForm(
        Player* player,
        std::string const& title,
        std::string const& content,
        std::function<void(Player* player, mce::UUID const& uuid)>&& callback) {
        return sendPlayerCategoryForm(player, "检查玩家背包", "选择玩家分类",
            [](Player* player, PlayerCategory category) {
                sendPlayerListForm(player, "检查玩家背包", "请选择要检查背包的玩家",
                    [](Player* player, mce::UUID const& uuid) {
                        auto result = CheckBagMgr.startCheckBag(player, uuid);
                        SendCheckResult(result, "开始检查玩家背包");
                    }, category);
            });;
    }

    //bool sendPlayerListFormSlow(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, mce::UUID const& uuid)>&& callback) {
    //    if (Config::GuiWithCategory) {
    //        Form::SimpleForm form(title, "请选择分类");
    //        auto classfiedlayerList = CheckBagMgr.getClassifiedPlayerList();
    //        for (auto& [category, list] : classfiedlayerList) {
    //            auto btn = Form::Button(toString(category));
    //            form.append(btn);
    //        }
    //        return form.sendTo((ServerPlayer*)player, [title, content, player, classfiedlayerList = std::move(classfiedlayerList), callback](int index) {
    //            if (index < 0)
    //                return;
    //            Form::SimpleForm form(title, content);
    //            std::vector<std::string> playerList = classfiedlayerList[index].second;
    //            for (auto& name : playerList) {
    //                auto btn = Form::Button(name);
    //                form.append(btn);
    //            }
    //            form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
    //                if (index < 0)
    //                    return;
    //                auto& target = playerList[index];
    //                auto uuid = mce::UUID::fromString(target);
    //                if (!uuid) {
    //                    auto suuid = PlayerInfo::getUUID(target);
    //                    uuid = mce::UUID::fromString(suuid);
    //                }
    //                callback(player, uuid);
    //            });
    //        });
    //    }
    //    else {
    //        Form::SimpleForm form(title, content);
    //        auto playerList = CheckBagMgr.getPlayerList();
    //        for (auto& name : playerList) {
    //            auto btn = Form::Button(name);
    //            form.append(btn);
    //        }
    //        return form.sendTo((ServerPlayer*)player, [player, playerList = std::move(playerList), callback](int index) {
    //            if (index < 0)
    //                return;
    //            auto& target = playerList[index];
    //            auto uuid = mce::UUID::fromString(target);
    //            if (!uuid) {
    //                auto suuid = PlayerInfo::getUUID(target);
    //                uuid = mce::UUID::fromString(suuid);
    //            }
    //            callback(player, uuid);
    //        });
    //    }
    //}

    bool sendDataTypeForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, NbtDataType type)>&& callback)
    {
        static std::vector<std::string> DataTypes = {
            toString(NbtDataType::Binary),
            toString(NbtDataType::Snbt),
            toString(NbtDataType::Json),
        };

        Form::SimpleForm form(title, content);
        for (auto& btn : DataTypes) {
            form.append(Form::Button{ btn });
        }
        return form.sendTo((ServerPlayer*)player, [player, callback](int index) {
            if (index < 0)
                return;
            auto dataType = fromString<NbtDataType>(DataTypes[index]);
            callback(player, dataType);
            });
    }

    bool openRemoveDataScreen(Player* player) {
        return sendPlayerListForm(player, "移除玩家数据", "请选择要移除数据的玩家",
            [](Player* player, mce::UUID const& uuid) {
                auto result = CheckBagMgr.removePlayerData(uuid);
                SendCheckResult(result, "移除玩家数据");
            });
    }

    bool openCheckBagMenuScreen(Player* player) {
        static std::vector<std::string> CheckBagMenus = {
            "查看其他玩家背包",
            "更新（自身背包 <= 目标玩家背包）",
            "覆盖（自身背包 => 目标玩家背包）",
            "查看下一个玩家",
            "停止查看玩家背包",
            "查看上一个玩家",
            "清除目标玩家所有数据",
        };

        auto& manager = CheckBagMgr;
        auto uuid = manager.tryGetTargetUuid(player);
        auto name = manager.getNameOrUuid(uuid);
        Form::SimpleForm form("检查玩家背包", fmt::format("当前玩家：{}\n请选择要执行的操作：", name));
        for (auto& btn : CheckBagMenus) {
            form.append(Form::Button(btn));
        }
        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            CheckBagManager::Result result = CheckBagManager::Result::Error;
            switch (do_hash(CheckBagMenus[index].c_str()))
            {
            case do_hash("查看其他玩家背包"):
                if (!openCheckBagScreen(player)) {
                    player->sendText("发送表单失败");
                }
                break;
            case do_hash("更新（自身背包 <= 目标玩家背包）"):
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                result = CheckBagMgr.startCheckBag(player, target);
                SendCheckResult(result, "开始查包");
                break;
            }
            case do_hash("覆盖（自身背包 => 目标玩家背包）"):
                result = CheckBagMgr.overwriteData(player);
                SendCheckResult(result, "覆盖玩家数据");
                break;
            case do_hash("停止查看玩家背包"):
                result = CheckBagMgr.stopCheckBag(player);
                SendCheckResult(result, "停止查包");
                break;
            case do_hash("清除目标玩家所有数据"): 
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                manager.stopCheckBag(player);
                auto result = manager.removePlayerData(target);
                SendCheckResult(result, "移除玩家数据");
                break;
            }
            case do_hash("查看下一个玩家"):
            {
                // 相对于所有玩家
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                auto targetName = manager.getNameOrUuid(target);
                auto list = manager.getPlayerList();
                auto iter = std::find(list.begin(), list.end(), targetName);
                ASSERT(iter != list.end());
                ++iter;
                if (iter == list.end())
                    iter = list.begin();
                auto result = manager.startCheckBag(player, mce::UUID::fromString(*iter));
                SendCheckResult(result, "查看下一个玩家");
                break;
            }
            case do_hash("查看上一个玩家"): 
            {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                auto targetName = manager.getNameOrUuid(target);
                auto list = manager.getPlayerList();
                auto iter = std::find(list.begin(), list.end(), targetName);
                ASSERT(iter != list.end());
                --iter;
                if (iter == list.begin())
                    iter = list.end();
                auto result = manager.startCheckBag(player, mce::UUID::fromString(*iter));
                SendCheckResult(result, "查看上一个玩家");
                break;
            }
            default:
                break;
            }
            });
    }

    bool openCheckBagScreen(Player* player) {
        if (Config::GuiWithCategory) {
            return sendPlayerListWithCategoryForm(player, "检查玩家背包", "选择玩家分类",
                [](Player* player, mce::UUID const& uuid) {
                    auto result = CheckBagMgr.startCheckBag(player, uuid);
                    SendCheckResult(result, "开始检查玩家背包");
                });
        }
        else {
            return sendPlayerListForm(player, "检查玩家背包", "请选择要检查背包的玩家",
                [](Player* player, mce::UUID const& uuid) {
                    auto result = CheckBagMgr.startCheckBag(player, uuid);
                    SendCheckResult(result, "开始检查玩家背包");
                });
        }
    }

    bool openCheckBagSmartScreen(Player* player) {
        if (CheckBagMgr.isCheckingBag(player))
            return openCheckBagMenuScreen(player);
        return openCheckBagScreen(player);
    }

    bool openExportScreen(Player* player) {
        return sendPlayerListForm(player, "导出玩家数据", "请选择要导出数据的玩家",
            [](Player* player, mce::UUID const& uuid) {
                sendDataTypeForm(player, "导出的数据类型", "",
                    [uuid](Player* player, NbtDataType dataType) {
                        auto result = CheckBagMgr.exportData(uuid, dataType);
                        SendCheckResult(result, "导出玩家数据");
                    });
            });
    }
    inline std::vector<std::string> listdir(std::string const& path) {
        if (!std::filesystem::exists(path))
            return {};
        if(std::filesystem::directory_entry(path).status().type() != std::filesystem::file_type::directory)
            return {};
        std::vector<std::string> listUuid;
        std::vector<std::string> listName;
        for (auto& file : std::filesystem::directory_iterator(path)) {
            auto& filePath = file.path();
            if (filePath.extension() == ".nbt"
                || filePath.extension() == ".snbt") {
                if(filePath.filename().string().find_last_of('.')==36)
                    listUuid.push_back(filePath.filename().string());
                else
                    listName.push_back(filePath.filename().string());
            }
        }
        std::sort(listUuid.begin(), listUuid.end());
        std::sort(listName.begin(), listName.end(), nameSortFunc);
        for (auto& suuid : listUuid) {
            listName.push_back(suuid);
        }
        return listName;
    }
    bool openImportScreen(Player* player) {
        //return false;
        std::vector<std::string> fileList = listdir(Config::ExportDirectory);
        Form::SimpleForm form("导入玩家数据","请选择数据");
        for (auto& btn : fileList) {
            form.append(Form::Button(btn));
        }
        return form.sendTo((ServerPlayer*)player,
            [player, fileList = std::move(fileList)](int index) {
                if (index < 0)
                    return;
                std::string filePath = fileList[index];
                std::string fileName = std::filesystem::path(filePath).filename().string();
                auto nameOrUuid = fileName.substr(0, fileName.find_last_of('.'));
                auto targetUuid = CheckBagManager::fromNameOrUuid(nameOrUuid);
                auto exist = !PlayerDataHelper::getServerId(targetUuid).empty();
                Form::CustomForm form("导入玩家数据");
                form.append(Form::Label("label", exist ? "发现匹配的玩家：" + nameOrUuid : "未找到匹配的玩家"));
                form.append(Form::Dropdown("importMode", "导入模式", { "仅背包","完整数据" }));
                form.append(Form::Dropdown("target", "导入目标",
                    exist ? std::vector<std::string>{ "匹配的玩家", "选择目标玩家" } : std::vector<std::string>{ "选择目标玩家" }));
                //form.append(Form::Dropdown("target", "导入目标", { exist ? "匹配的玩家" : "新玩家", "选择目标玩家" }));
                form.sendTo((ServerPlayer*)player, [player, filePath, targetUuid](const std::map<string, std::shared_ptr<Form::CustomFormElement>>& data) {
                    auto modeDW = std::dynamic_pointer_cast<Form::Dropdown>(data.at("importMode"));
                    auto isBagOnly = modeDW->options[modeDW->getData()] == "仅背包";
                    auto targetDW = std::dynamic_pointer_cast<Form::Dropdown>(data.at("target"));
                    auto& target = targetDW->options[targetDW->getData()];
                    switch (do_hash(target.c_str()))
                    {
                    case do_hash("匹配的玩家"):
                        if (getPlayer(targetUuid) && !isBagOnly) {
                            player->sendText(fmt::format("§c§l玩家在线时不能进行完整数据导入§r"));
                            return;
                        }
                        else {
                            auto result = CheckBagMgr.importData(targetUuid, filePath, isBagOnly);
                            SendCheckResult(result, "导入玩家数据");
                        }
                        break;
                    case do_hash("新玩家"):
                        player->sendText(fmt::format("§c§l未实现§r"));
                        return;
                        if (isBagOnly)
                        {
                            player->sendText(fmt::format("§c§l仅背包模式下不可选择新玩家作为目标§r"));
                        }
                        else {
                            if (!std::filesystem::exists(filePath + ".json")) {
                                player->sendText(fmt::format("§c§l获取玩家信息文件失败，文件 {} 不存在§r", filePath + ".json"));
                                return;
                            }
                            
                            auto data = ReadAllFile(filePath, true);
                            if (!data.has_value()) {
                                player->sendText(fmt::format("§c§l读取数据失败§r"));
                                return;
                            }
                            auto tag = CompoundTag::fromBinaryNBT((void*)data.value().c_str(), data.value().size(), true);
                            //auto playerInfo = ReadAllFile()
                            //PlayerDataHelper::writeNewPlayerData()
                        }
                        break;
                    case do_hash("选择目标玩家"):
                        sendPlayerListForm(player, "导入玩家数据", "选择要导入数据的目标玩家", 
                            [filePath, isBagOnly](Player* player, mce::UUID const& uuid) {
                                auto result = CheckBagMgr.importData(uuid, filePath, isBagOnly);
                                SendCheckResult(result, "导入玩家数据");
                            });
                        break;
                    default:
                        break;
                    }
                    

                    });
            });
    }

    bool openExportAllScreen(Player* player) {
        return sendDataTypeForm(player, "导出所有玩家数据", "要导出的数据类型",
            [](Player* player, NbtDataType type) {
                auto& manager = CheckBagMgr;
                size_t count = 0;
                for (auto& suuid : manager.getPlayerList()) {
                    auto result = manager.exportData(suuid, type);
                    if (result == CheckBagManager::Result::Success)
                        count++;
                    else {
                        player->sendText(fmt::format("§c§l导出 {} 数据失败§r", suuid));
                        player->sendText(std::string("§c§l原因：") + CheckBagManager::getResultString(result) + "§r");
                    }
                }
                player->sendText(fmt::format("成功导出 {} 个玩家的数据", count));
            });
    }

    bool openMenuScreen(Player* player) {
        static std::vector<std::string> MenuButtons{
            toString(ScreenCategory::Check),
            //toString(ScreenCategory::Menu),
            toString(ScreenCategory::Import),
            toString(ScreenCategory::Export),
            toString(ScreenCategory::Delete),
        };

        auto& manager = CheckBagMgr;
        Form::SimpleForm form("LLCheckBag", "请选择要执行的操作");
        for (auto& btn : MenuButtons) {
            form.append(Form::Button{ btn });
        }
        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            switch (fromString<ScreenCategory>(MenuButtons[index]))
            {
            case ScreenCategory::Check:
                openCheckBagSmartScreen(player);
                break;
            case ScreenCategory::Menu:
                openMenuScreen(player);
                break;
            case ScreenCategory::Import:
                openImportScreen(player);
                break;
            case ScreenCategory::Export:
                openExportScreen(player);
                break;
            case ScreenCategory::Delete:
                openRemoveDataScreen(player);
                break;
            case ScreenCategory::ExportAll: {
                openExportAllScreen(player);
                break;
            }
            default:
                break;
            }
            });
    }
}