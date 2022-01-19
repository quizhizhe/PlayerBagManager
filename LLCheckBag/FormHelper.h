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
	player->sendText(operation "失败");\
	player->sendText("原因" + CheckBagManager::getResultString(result));\
}

namespace FormHelper {
    bool sendPlayerListForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, mce::UUID const& uuid)>&& callback);
    bool sendDataTypeForm(Player* player, std::string const& title, std::string const& content, std::function<void(Player* player, NbtDataType type)>&& callback);
    bool openRemoveDataScreen(Player* player);
    bool openCheckBagScreen(Player* player);
    bool openExportScreen(Player* player);
    bool openImportScreen(Player* player);
    bool openMenuScreen(Player* player);
}