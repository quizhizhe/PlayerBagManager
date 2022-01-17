#pragma once
#include <MC/Player.hpp>

bool sendPlayerList(Player* player, const std::vector<string> playerList, std::function<void(string)> callback);
//int sendSimpleForm(Player* player, const string& title, const string& content, const std::vector<string>& buttons);