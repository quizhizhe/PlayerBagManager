# LLCheckBag-1.16.40
This is the inventory check plug-in for [Bedrock Minecraft Server](https://www.minecraft.net/en-us/download/server/bedrock), 
which can check the plug-ins of all players' inventory in the Level, and comes with a variety of player data operations to confirm whether the player data is abnormal. 
[LiteLoader](https://github.com/LiteLDev/LiteLoaderBDS) is required beforehand.

> This page is translated by Google Translation and may need to be optimized 

##### [简体中文](https://github.com/quizhizhe/LLCheckBag#readme) | English

## Features
* View a player's items, include Inventory, EnderChestInventory, Armor, Offhand items(Inventory below refers to these containers)
    * Principle: Back up the data of the operating player, and set the Inventory data of the target player to the operating player
* Export a player's data
* Import a player's data
* Delete all data of a player, **not just Inventory data**
> Note: The above functions support all players in the Level. In theory, the name of players who have entered the server after installing LiteLoader 2.0 on the server can be recognized , otherwise it will be displayed in the form of uuid

## Download and install
> *Make sure you have the [LiteLoader-1.16.40](https://github.com/quizhizhe/LiteLoaderBDS-1.16.40) installed correctly*
* Download binary
     * It can be downloaded [Github Release](https://github.com/quizhizhe/LLCheckBag/releases). After completion, unzip the contents of the compressed package to the plugins directory.
* Compile from source
     * Use `git clone -b 1.16.40 https://github.com/quizhizhe/LLCheckBag.git` to pull the project, open the sln file with [Visual Studio 2022](https://visualstudio.microsoft.com/vs/), Set the project debug path to the bds path and generate it

## Usage
* Command(*You can replace llcheckbag with the set command alias, the default is llcb*)
**For the first time, please use the `llcheckbag op your xboxid` command in the BDS console to set you as OP, otherwise you will be prompted that you do not have permission**
> Note: If the target player `[Target]` is not set, it will open the corresponding GUI
```
llcheckbag                              // Open the default GUI screen (the GUI set by DefaultScreen in the configuration file)
llcheckbag check/c      [Target]        // Check player's Inventory
llcheckbag remove/rm    [Target]        // remove player data
llcheckbag op           [Target]        // Set up a plugin manager
llcheckbag export/e     [Target] [Type] // Export player data
llcheckbag exportall    [Type]          // Export all player data, **not just Inventory data**
llcheckbag rollback/rb                  // Roll back player data (usually used after the server crashes when checking packages)
llcheckbag overwrite/ow                 // Overwrite the checked player Inventory
llcheckbag stop/s                       // Stop checking
llcheckbag menu/m                       // Open the Inventory check GUI menu
llcheckbag list/l                       // List all players (player name or player UUID)
llcheckbag import/i                     // Import player data (GUI only)
llcheckbag next/n                       // Check next player Inventory (relative to all players)
llcheckbag previous                     // Check previous player Inventory (relative to all players)
```

## Part of the GUI usage
### Import
* Enter `llcheckbag import` or `llcheckbag i` to open the import screen, select the player data to be imported,
* The files in the list are the files in the folder set by **ExportDirectory**, only the files with the suffix `.snbt` or `.nbt` are supported
After selecting, the **Import data screen** will pop up.
* The first line shows the auto-matched player name already in the Level,
* Import mode is divided into Inventory only and full data mode,
     * Inventory only: This option will import Inventory item data
     * Full data: This option will import full player nbt data
* The import targets are divided into matched players, new players, and selected players. **It should be noted that the new player mode requires a player information file (suffixed with `.nbt.json` or `.snbt.json`)**

## Configuration file(plugins/LLCheckBag/config.json)
```javascript
{
    // Configuration file version, in order to convert the configuration to the latest version, **Do Not Modify**
    "ConfigVersion": 1,
    // Backup data format, optional values: Binary, Snbt
    "BackupDataType": "Binary",
    // Inventory backup directory
    "BackupDirectory": "plugins/LLCheckBag/Backup/",
    // Command alias, you can use the alias to execute the command when entering the command, similar to the /teleport alias /tp
    "CommandAlias": "llcb",
    // Whether to enable custom operators
    "CustomOperator": true,
    // Set the default gui menu, optional values: Check, Menu, Import, Export, Delete, ExportAll
    "DefaultScreen": "Check",
    // The export directory is also the import directory
    "ExportDirectory": "plugins/LLCheckBag/Export/",
    // Output formatted Snbt instead of minimized Snbt
    "FormattedSNBT": true,
    // language, optional value: zh_CN, en_US
    "Language": "zh_CN",
    // If you don't understand, please choose the default value
    "MsaIdOnly": false,
    // Only valid when CustomOperator is true, set the xuid of the player who has permission to use the command (string format)
    "OperatorXuidList": []
}
```

## Remark
* If you need to edit and import player data, please make sure you have a certain degree of understanding of nbt data format and player data structure, and **please backup your Level first**
* It is recommended to use [nbt-studio](https://github.com/tryashtar/nbt-studio) or other nbt editing tools to view or edit the exported player nbt data (in other words, do not use Microsoft Notepad)
