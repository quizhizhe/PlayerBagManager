#pragma once
#include <MC/Types.hpp>
#include <MC/Player.hpp>
#include <FormUI.h>
#include "CheckBagManager.h"
#include <PlayerInfoAPI.h>

#define SendCheckResult(result, operation)\
if (result == CheckBagManager::Result::Success)\
	player->sendText(tr("operation.success", operation));\
else {\
	player->sendText(fmt::format("§c§l{}§r", tr("operation.failed", operation, CheckBagManager::getResultString(result)))); \
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
    bool openExportAllScreen(Player* player);
    bool openMenuScreen(Player* player);
}