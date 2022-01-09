#pragma once
#include "pch.h"

struct UuidStruct
{
	string name;
	string xuid;
	string serverId;
};
bool loadUuiddb();
string findUuidByName(const string& name);
string findUuidByXuid(const string& xuid);
string findUuid(const string& key);
string getXuidByUuid(string& uuid);
string getNameByUuid(string& uuid);
std::vector<UuidStruct> getAllUuidData();
bool tryToSaveUuid(string& uuid, UuidStruct& uuidStruct);
