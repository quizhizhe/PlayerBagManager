# LLCheckBag
这是 [Bedrock Minecraft Server](https://www.minecraft.net/en-us/download/server/bedrock) 的背包检查插件，
可以查看存档中所有玩家背包的插件，并附带多种玩家数据操作用于确认玩家数据是否异常
需要 [LiteLoader](https://github.com/LiteLDev/LiteLoaderBDS) 的前置

##### 简体中文 | [English](README_en.md)

## 功能
* 查看某玩家背包
* 导出某玩家数据
* 导入某玩家数据
* 删除某玩家所有数据，注：**不只是背包数据**
    * 注：以上功能均支持存档中所有玩家，理论上服务器安装 LiteLoader 2.0 后进入过服务器的玩家均能识别出玩家名，否则以uuid形式显示

## 编译
使用
``
git clone --recursive https://github.com/quizhizhe/LLCheckBag.git
``
拉取项目，用 [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) 打开sln文件，选择，生成->生成LLCheckBag

## 用法
* 指令（*可将 llcheckbag 替换成设置的指令别名，默认 llcb*）：
* 注：如果目标玩家`[Target]`未设置则为打开对应 GUI
```
llcheckbag
llcheckbag check/c      [Target]        // 检查玩家背包
llcheckbag remove/rm    [Target]        // 移除玩家数据
llcheckbag op           [Target]        // 设置查包管理员
llcheckbag export/e     [Target] [Type] // 导出玩家数据
llcheckbag exportall    [Type]          // 导出所有玩家数据，**不只是背包数据**
llcheckbag rollback/rb                  // 回滚玩家数据（一般查包时崩服后用）
llcheckbag overwrite/ow                 // 覆盖被查包玩家数据
llcheckbag stop/s                       // 停止查包
llcheckbag menu/m                       // 打开查包 GUI 菜单
llcheckbag list/l                       // 列出所有玩家（玩家名称或者玩家UUID）
llcheckbag import/i                     // 导入玩家数据（仅GUI）
```

## 部分 GUI 说明
### Import 导入玩家数据
* 输入 `llcheckbag import` 或 `llcheckbag i` 可打开导入界面，选择需要导入的玩家数据，
* 列表里文件为 **ExportDirectory** 设置的文件夹中的文件，仅支持后缀为 `.snbt` 或者 `.nbt` 的文件
选择好后会弹出**导入数据界面**，
* 第一行显示自动的是自动匹配的存档中已有的玩家名，
* 导入模式分为仅背包和完整数据模式，
    * 仅背包：此选项会会导入背包物品数据，包括 人物背包，末影箱，盔甲蓝，副手，玩家UI物品数据
    * 完整数据：此选项会导入完整的玩家nbt数据
* 导入目标分为匹配的玩家，新玩家，选择玩家，**需要注意的是，新玩家模式需要玩家信息文件（后缀为`.nbt.json`或者`.snbt.json`）**

## 配置文件(plugins/LLCheckBag/config.json)
```json
{
    // 配置文件版本，为了转换旧版本的配置到最新版本，**别修改**
    "ConfigVersion": 1,
    // 备份数据格式，可选值：Binary, Snbt
    "BackupDataType": "Binary",
    // 背包备份目录
    "BackupDirectory": "plugins/LLCheckBag/Backup/",
    // 指令别名，输入指令时可用别名执行指令，类似 /teleport 的别名 /tp
    "CommandAlias": "llcb",
    // 是否启用自定义操作者
    "CustomOperator": true,
    // 设置默认的 gui 菜单，可选值：Check, Menu, Import, Export, Delete, ExportAll
    "DefaultScreen": "Check",
    // 导出目录，同时也是导入目录
    "ExportDirectory": "plugins/LLCheckBag/Export/",
    // 语言，可选值：zh_CN, en_US
    "Language": "zh_CN",
    // 不懂请选择默认值
    "MsaIdOnly": false,
    // 仅 CustomOperator 为 true 时有效，设置权限使用指令的玩家的xuid（字符串格式）
    "OperatorXuidList": []
}
```

## 备注
* 如果需要编辑并导入玩家数据请确保你对 nbt 数据格式以及玩家数据结构有一定程度的了解，并**请先备份好你的存档**
* 建议使用 [nbt-studio](https://github.com/tryashtar/nbt-studio) 或其他 nbt 编辑工具查看或者编辑导出的玩家 nbt 数据（其实就是千万别用记事本）