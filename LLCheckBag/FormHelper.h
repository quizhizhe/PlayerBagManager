#pragma once
#include <MC/Types.hpp>
#include <MC/Player.hpp>
#include <FormUI.h>
#include "CheckBagManager.h"
#include <PlayerInfoAPI.h>

#define CheckResultSend(result, operation)\
if (result == CheckBagManager::Result::Success)\
	player->sendText(operation "成功");\
else {\
	player->sendText(std::string("§c§l") + operation + "失败§r");\
	player->sendText(std::string("§c§l原因：") + CheckBagManager::getResultString(result)+"§r");\
}

namespace FormHelper {
    bool sendPlayerListForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, mce::UUID const& uuid)>&& callback, PlayerCategory category = PlayerCategory::All);
    bool sendDataTypeForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, NbtDataType type)>&& callback);
    bool openRemoveDataScreen(Player* player);
    bool openCheckBagMenuScreen(Player* player);
    bool openCheckBagScreen(Player* player);
    bool openCheckBagSmartScreen(Player* player);
    bool openExportScreen(Player* player);
    bool openImportScreen(Player* player);
    bool openMenuScreen(Player* player);
}