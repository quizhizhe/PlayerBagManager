#include "pch.h"
#include "SymHelp.h"


vector<string> loadAllPlayerIDs() {
	vector<string> playerIDList;
	vector<string> playerIDListTemp = ::Global<LevelStorage>->loadAllPlayerIDs(false);
	for (string data : playerIDListTemp) { //remove have "server_" valve
		string::size_type idx = data.find("server_");
		if (idx != string::npos) {
			continue;
		}
		else {
			playerIDList.push_back(data);
		}
	}
	return playerIDList;
}