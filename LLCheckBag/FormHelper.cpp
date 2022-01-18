#include "pch.h"
#include <map>
#include "FormHelper.h"
#include <MC/Packet.hpp>
namespace FormHelper {
    bool sendPlayerListForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, mce::UUID const& uuid)>&& callback) {
        Form::SimpleForm form(title, content);
        auto playerList = CheckBagMgr.getPlayerList();
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

    bool openCheckBagScreen(Player* player) {
        return sendPlayerListForm(player, "检查玩家背包", "请选择要检查背包的玩家",
            [](Player* player, mce::UUID const& uuid) {
                auto result = CheckBagMgr.startCheckBag(player, uuid);
                CheckResultSend(result, "开始检查玩家背包");
            });
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
                openCheckBagScreen(player);
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