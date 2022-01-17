#include "pch.h"
#include <map>
#include "FormHelper.h"
#include <MC/Packet.hpp>

std::map<unsigned, std::function<void(string)>> formCallbacks;


// ===== sendForm =====
Packet* createPacket(MinecraftPacketIds type)
{
	unsigned long long packet[2] = { 0 };
	SymCall("?createPacket@MinecraftPackets@@SA?AV?$shared_ptr@VPacket@@@std@@W4MinecraftPacketIds@@@Z",
		void*, void*, MinecraftPacketIds)(packet, type);
	return (Packet*)*packet;
}

int sendForm(Player* player, const string& data) {
	unsigned formId = (unsigned)((rand() << 16) + rand());
	Packet* packet = createPacket(MinecraftPacketIds::ModalFormRequest);   //表单数据包
	dAccess<unsigned>(packet, 48) = formId;
	dAccess<string>(packet, 56) = data;
	((ServerPlayer*)player)->sendNetworkPacket(*(Packet*)packet);
	std::cout << formId << ": " << data << std::endl;
	return formId;
}

int sendSimpleForm(Player* player, const string& title, const string& content, const std::vector<string>& buttons)
{
	string model = u8R"({"title":"%s","content":"%s","buttons":%s,"type":"form"})";
	model = model.replace(model.find("%s"), 2, title);
	model = model.replace(model.find("%s"), 2, content);
	string strButtons = "[";
	for (int i = 0; i < buttons.size(); ++i)
	{
		strButtons += "{\"text\":\""+buttons[i]+"\"}";
		if (i != buttons.size() - 1) {
			strButtons += ",";
		}
	}
	strButtons += "]";
	model = model.replace(model.find("%s"), 2, strButtons);
	return sendForm(player, model);
}


bool sendPlayerList(Player* player, const std::vector<string> playerList, std::function<void(string)> callback) {
	unsigned formId = sendSimpleForm(player, "LL Check Bag", "LL Check Bag", playerList);
	formCallbacks[formId] = callback;
	return true;
}

// ===== handleCallBack =====
bool callFormCallback(Player* player, unsigned formId, const string& data)
{
	auto cb = formCallbacks.find(formId);
	if (cb != formCallbacks.end()) {
		cb->second(data);
		formCallbacks.erase(cb);
	}
}
Player* getPlayerFromPacket(ServerNetworkHandler* handler, NetworkIdentifier* id, Packet* packet)
{
	return SymCall("?_getServerPlayer@ServerNetworkHandler@@AEAAPEAVServerPlayer@@AEBVNetworkIdentifier@@E@Z",
		Player*, ServerNetworkHandler*, NetworkIdentifier*, char)(handler, id, dAccess<char>(packet, 16));
}

THook(void, "?handle@?$PacketHandlerDispatcherInstance@VModalFormResponsePacket@@$0A@@@UEBAXAEBVNetworkIdentifier@@AEAVNetEventCallback@@AEAV?$shared_ptr@VPacket@@@std@@@Z",
    void* _this, NetworkIdentifier* id, ServerNetworkHandler* handler, void* pPacket)
{
        Packet* packet = *(Packet**)pPacket;
        Player* player = getPlayerFromPacket(handler, id, packet);

        if (player)
        {
            unsigned formId = dAccess<unsigned>(packet, 48);
            string data = dAccess<string>(packet, 56);

            if (data.back() == '\n')
                data.pop_back();

            callFormCallback(player, formId, data);
        }

    original(_this, id, handler, pPacket);
}
