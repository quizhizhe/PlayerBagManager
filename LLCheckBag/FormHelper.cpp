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
            auto btn = Form::Button(name);
            form.append(btn);
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
        Form::SimpleForm form(title, "请选择分类");

        form.append(Form::Button(toString(PlayerCategory::All)));
        form.append(Form::Button(toString(PlayerCategory::Normal)));
        form.append(Form::Button(toString(PlayerCategory::FakePlayer)));
        form.append(Form::Button(toString(PlayerCategory::Unnamed)));

        return form.sendTo((ServerPlayer*)player, [title, content, player, callback](int index) {
            if (index < 0)
                return;
            auto category = (PlayerCategory)index;
            callback(player, category);
            });
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
        Form::SimpleForm form(title, content);
        form.append(Form::Button("Snbt")).append(Form::Button("Binary")).append(Form::Button("Json"));
        return form.sendTo((ServerPlayer*)player, [player, callback](int index) {
            if (index < 0)
                return;
            callback(player, (NbtDataType)index);
            });
    }

    bool openRemoveDataScreen(Player* player) {
        return sendPlayerListForm(player, "移除玩家数据", "请选择要移除数据的玩家",
            [](Player* player, mce::UUID const& uuid) {
                auto result = CheckBagMgr.removePlayerData(uuid);
                CheckResultSend(result, "移除玩家数据");
            });
    }

    bool openCheckBagMenuScreen(Player* player) {
        Form::SimpleForm form("检查玩家背包", "请选择要执行的操作");
        std::vector<std::string> buttonList;
        form.append(Form::Button("查看其他玩家背包"));
        form.append(Form::Button("更新目标玩家背包"));
        form.append(Form::Button("覆盖目标玩家背包"));
        form.append(Form::Button("停止查看玩家背包"));
        form.append(Form::Button("清除目标玩家所有数据"));

        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            CheckBagManager::Result result = CheckBagManager::Result::Error;
            switch (index)
            {
            case 0:
                if (!openCheckBagScreen(player)) {
                    player->sendText("发送表单失败");
                }
                break;
            case 1: {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                result = CheckBagMgr.startCheckBag(player, target);
                CheckResultSend(result, "开始查包");
                break;
            }
            case 2:
                result = CheckBagMgr.overwriteData(player);
                CheckResultSend(result, "覆盖玩家数据");
                break;
            case 3:
                result = CheckBagMgr.stopCheckBag(player);
                CheckResultSend(result, "停止查包");
                break;
            case 4: {
                auto& manager = CheckBagMgr;
                auto target = manager.tryGetTargetUuid(player);
                manager.stopCheckBag(player);
                auto result = manager.removePlayerData(target);
                CheckResultSend(result, "移除玩家数据");
                break;
            }
            default:
                break;
            }
            });
    }

    bool openCheckBagScreen(Player* player) {
        if (Config::GuiWithCategory) {
            return sendPlayerCategoryForm(player, "检查玩家背包", "选择玩家分类",
                [](Player* player, PlayerCategory category) {
                    sendPlayerListForm(player, "检查玩家背包", "请选择要检查背包的玩家",
                        [](Player* player, mce::UUID const& uuid) {
                            auto result = CheckBagMgr.startCheckBag(player, uuid);
                            CheckResultSend(result, "开始检查玩家背包");
                        }, category);
                });
        }
        else {
            return sendPlayerListForm(player, "检查玩家背包", "请选择要检查背包的玩家",
                [](Player* player, mce::UUID const& uuid) {
                    auto result = CheckBagMgr.startCheckBag(player, uuid);
                    CheckResultSend(result, "开始检查玩家背包");
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
                        CheckResultSend(result, "导出玩家数据");
                    });
            });
    }

    bool openImportScreen(Player* player) {
        return false;
        return sendPlayerListForm(player, "导入玩家数据", "请选择要导出数据的玩家",
            [](Player* player, mce::UUID const& uuid) {
                sendDataTypeForm(player, "导入的数据类型", "",
                    [uuid](Player* player, NbtDataType dataType) {
                        //auto result = importData(uuid, dataType);
                        //CheckResultSendGUI(result, "导入玩家数据");
                    });
            });
    }

    bool openMenuScreen(Player* player) {
        auto& manager = CheckBagMgr;
        Form::SimpleForm form("LLCheckBag", "请选择要执行的操作");
        form.append(Form::Button(toString(ScreenCategory::Check)));
        form.append(Form::Button(toString(ScreenCategory::Menu)));
        form.append(Form::Button(toString(ScreenCategory::Import)));
        form.append(Form::Button(toString(ScreenCategory::Export)));
        form.append(Form::Button(toString(ScreenCategory::Delete)));

        return form.sendTo((ServerPlayer*)player, [player](int index) {
            if (index < 0)
                return;
            switch ((ScreenCategory)index)
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
            default:
                break;
            }
            });
    }
}