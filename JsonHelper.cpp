#include "pch.h"
#include <filesystem>
#include "JsonHelper.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace std;
using namespace rapidjson;

constexpr auto UUIDDB_PATH = "./plugins/LLCheckBag/uuidDat.json";
Document uuiddb;


bool writeUuiddb() {
	if (uuiddb.IsNull()) {
		uuiddb.Parse("{}");
		return false;
	}
	filesystem::create_directory("./plugins/LLCheckBag");
	ofstream of(UUIDDB_PATH);
	if (of) {
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		uuiddb.Accept(writer);
		of << buffer.GetString();
		of.flush();
		of.close();
		return true;
	}
	else {
		cerr << "[LLCheckBag] 配置文件读取或创建失败" << endl;
		return false;
	}
	return false;
}

bool loadUuiddb() {
	ifstream fs;
	fs.open(UUIDDB_PATH, ios::in);
	if (!fs) {
		cout << "[LLCheckBag] 没找到 " << UUIDDB_PATH << " 文件，即将创建新数据文件" << endl;
		uuiddb.Parse("{}");
		return writeUuiddb();
	}
	else {
		cout << "[LLCheckBag] 正在加载数据文件 " << UUIDDB_PATH << endl;
		string json;
		char buf[1024];
		while (fs.getline(buf, 1024)) {
			json.append(buf);
		}
		uuiddb.Parse(json.c_str());
		fs.close();
		return true;
	}
	if (uuiddb.IsNull()) {
		uuiddb.Parse("{}");
	}
}

string findUuidByName(const string& name) {
	for (auto uuid = uuiddb.MemberBegin(); uuid != uuiddb.MemberEnd(); ++uuid) {
		auto nameKey = uuid->value.FindMember("name");
		if (nameKey != uuid->value.MemberEnd() && nameKey->value.GetString() == name) {
			return uuid->name.GetString();
		}
	}
	return "";
}

string findUuidByXuid(const string& xuid) {
	for (auto uuid = uuiddb.MemberBegin(); uuid != uuiddb.MemberEnd(); ++uuid) {
		auto xuidKey = uuid->value.FindMember("xuid");
		if (xuidKey != uuid->value.MemberEnd() && xuidKey->value.GetString() == xuid) {
			return uuid->name.GetString();
		}
	}
	return "";
}

string findUuid(const string& key) {
	for (auto uuidKey = uuiddb.MemberBegin(); uuidKey != uuiddb.MemberEnd(); ++uuidKey) {
		auto nameKey = uuidKey->value.FindMember("name");
		if (nameKey != uuidKey->value.MemberEnd() && nameKey->value.GetString() == key) {
			return uuidKey->name.GetString();
		}
		auto xuidKey = uuidKey->value.FindMember("xuid");
		if (xuidKey != uuidKey->value.MemberEnd() && xuidKey->value.GetString() == key) {
			return uuidKey->name.GetString();
		}
	}
	return "";
}

string getXuidByUuid(string& uuid) {
	auto key = uuiddb.FindMember(uuid.c_str());
	if (key != uuiddb.MemberEnd()) {
		auto xuidKey = key->value.FindMember("xuid");
		if (xuidKey != key->value.MemberEnd()) {
			return xuidKey->value.GetString();
		}
	}
	return "";
}
string getNameByUuid(string& uuid) {
	auto key = uuiddb.FindMember(uuid.c_str());
	if (key != uuiddb.MemberEnd()) {
		auto nameKey = key->value.FindMember("name");
		if (nameKey != key->value.MemberEnd()) {
			return nameKey->value.GetString();
		}
	}
	return "";
}

UuidStruct* getUuidStruct(string& uuid) {
	auto key = uuiddb.FindMember(uuid.c_str());
	if (key != uuiddb.MemberEnd()) {
		UuidStruct uuidStruct;
		auto nameKey = key->value.FindMember("name");
		if (nameKey != key->value.MemberEnd()) {
			uuidStruct.name = nameKey->value.GetString();
		}
		auto xuidKey = key->value.FindMember("xuid");
		if (xuidKey != key->value.MemberEnd()) {
			uuidStruct.xuid = xuidKey->value.GetString();
		}
		auto serveridKey = key->value.FindMember("serverId");
		if (serveridKey != key->value.MemberEnd()) {
			uuidStruct.serverId = serveridKey->value.GetString();
		}
		return &uuidStruct;
	}
	return nullptr;
}

std::vector<UuidStruct> getAllUuidData() {
	vector<UuidStruct> allData;
	for (auto uuid = uuiddb.MemberBegin(); uuid != uuiddb.MemberEnd(); ++uuid) {
		UuidStruct us;
		us.name = uuid->value["name"].GetString();
		us.xuid = uuid->value["xuid"].GetString();
		us.serverId = uuid->value["serverId"].GetString();
		allData.push_back(us);
	}
	return allData;
}

bool addNewUuid(string& uuid, UuidStruct& uuidStruct) {
	Document::AllocatorType& allocator = uuiddb.GetAllocator();
	Value value(kObjectType);

	//uuiddb[uuid.c_str()].SetObject();
	//auto& all = uuiddb.GetAllocator();
	Value name(kStringType);
	name.SetString(uuidStruct.name.c_str(), uuidStruct.name.size(), allocator);
	value.AddMember("name", name, allocator);
	Value xuid(kStringType);
	xuid.SetString(uuidStruct.xuid.c_str(), uuidStruct.xuid.size(), allocator);
	value.AddMember("xuid", xuid, allocator);
	Value serverId(kStringType);
	serverId.SetString(uuidStruct.serverId.c_str(), uuidStruct.serverId.size(), allocator);
	value.AddMember("serverId", serverId, allocator);
	Value key(kStringType);
	key.SetString(uuid.c_str(), allocator);
	uuiddb.AddMember(key, value, allocator);
	//cout << uuiddb[key]["name"].GetString() << endl;
	return writeUuiddb();
}

bool tryToSaveUuid(string& uuid, UuidStruct& uuidStruct) {
	auto key = uuiddb.FindMember(uuid.c_str());
	if (key != uuiddb.MemberEnd()) {
		bool dirty = false;
		auto nameKey = key->value.FindMember("name");
		if (nameKey != key->value.MemberEnd()) {
			if (uuidStruct.name != nameKey->value.GetString()) {
				dirty = true;
				Value& val = uuiddb[uuid.c_str()]["name"];
				val.SetString(uuidStruct.name.c_str(), uuiddb.GetAllocator());
			}
		}
		auto xuidKey = key->value.FindMember("xuid");
		if (xuidKey != key->value.MemberEnd()) {
			if (uuidStruct.xuid != xuidKey->value.GetString()) {
				dirty = true;
				Value& val = uuiddb[uuid.c_str()]["xuid"];
				val.SetString(uuidStruct.xuid.c_str(), uuiddb.GetAllocator());
			}
		}
		auto serveridKey = key->value.FindMember("serverId");
		if (serveridKey != key->value.MemberEnd()) {
			if (uuidStruct.serverId != serveridKey->value.GetString()) {
				dirty = true;
				Value& val = uuiddb[uuid.c_str()]["serverId"];
				val.SetString(uuidStruct.serverId.c_str(), uuiddb.GetAllocator());
			}
		}
		if (dirty) {
			return writeUuiddb();
		}
		else {
			return true;
		}

	}
	else {
		return addNewUuid(uuid, uuidStruct);
	}
	return false;
}