#include "PlayerBagManagerCommand.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/command/runtime/RuntimeOverload.h"
#include "ll/api/service/Bedrock.h"

#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/world/events/ServerInstanceEventCoordinator.h"
#include "mc/server/commands/CommandSelector.h"
#include "mc/server/commands/CommandSelectorResults.h"



namespace player_bag_manager::command {
using namespace ll::i18n_literals;

void UpdatePlayerLstSoftEnum(const std::vector<std::string>& playerlist) {
        ll::service::getCommandRegistry()->setSoftEnumValues("PlayerBagManager_PlayerList", playerlist);
}

struct ParamOperation {

    enum DataOperation : int { Export, Import, Remove } dataOperation;

    enum BagOperation : int {
        Menu,
        Check,
        StopCheck,
        List,
        Next,
        Previous,
        Overwrite
    } bagOperation;

    CommandSelector<Player> player;

};

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "playerbag",
        "Player bag manager main command",
        CommandPermissionLevel::Admin
    );
    static auto bagLambda = [](CommandOrigin const& origin, CommandOutput& output, ParamOperation const& param) {
        if(param.player.results(origin).empty()) {
            output.error("Not player target");
        } else {
            output.success("{} player has target", param.player.results(origin).size());
        };
        switch (param.bagOperation) {
            case ParamOperation::BagOperation::Menu: {
                // 主菜单表单
                output.success("menu");
                break;
            }
            case ParamOperation::BagOperation::Check: {
                // 选择了玩家查包
                output.success("check");
                break;
            }
            case ParamOperation::BagOperation::StopCheck: {
                // 停止，恢复背包
                output.success("stop check");
                break;
            }
            case ParamOperation::BagOperation::List: {
                // 玩家背包列表表单
                output.success("list bag");
                break;
            }
            case ParamOperation::BagOperation::Next: {
                // 处于查包时，选择下一个
                output.success("next bag");
                break;
            }
            case ParamOperation::BagOperation::Previous: {
                // 处于查包时，选择上一个
                output.success("previous");
                break;
            }
            case ParamOperation::BagOperation::Overwrite: {
                // 覆盖正在查看的玩家的背包
                output.success("overwrite");
                break;
            }
            default: {
                // menu
                output.success("default");
            }
        }
    };
    static auto dataLambda = [](CommandOrigin const& origin, CommandOutput& output, ParamOperation const& param) {
        if(param.player.results(origin).empty()) {
            output.error("Not player target");
        } else {
            output.success("{} player has target", param.player.results(origin).size());
        };
        switch (param.dataOperation) {
            case ParamOperation::DataOperation::Export: {
                // 导出选择的玩家
                output.success("export data");
                break;
            }
            case ParamOperation::DataOperation::Import: {
                // 导入
                output.success("import data");
                break;
            }
            case ParamOperation::DataOperation::Remove: {
                // 删除
                output.success("remove data");
                break;
            }
            default: {
                // menu
                output.success("default");
            }
        }
    };
    cmd.alias("pbm");
    cmd.overload<ParamOperation>()
    .text("bag")
    .required("bagOperation")
    .optional("player")
    .execute(bagLambda);

    cmd.overload<ParamOperation>()
    .text("data")
    .required("dataOperation")
    .optional("player")
    .execute(dataLambda);

    cmd.overload<ParamOperation>()
    .text("data")
    .text("export_all")
    .execute([](CommandOrigin const& ori, CommandOutput& output, ParamOperation const& param) {
        // 导出所有
    });


}

LL_TYPE_INSTANCE_HOOK(
    registerBuiltinCommands,
    ll::memory::HookPriority::Normal,
    ServerInstanceEventCoordinator,
    &ServerInstanceEventCoordinator::sendServerThreadStarted,
    void,
    ::ServerInstance& ins
) {
    origin(ins);
    registerCommand();
}

ll::memory::HookRegistrar<registerBuiltinCommands> hooks{};

} // namespace player_bag_manager::command