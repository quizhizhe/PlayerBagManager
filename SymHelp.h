#pragma once
#include <MC/Level.hpp>
#include <MC/LevelStorage.hpp>

// Call
//template<typename RTN = void, typename... Args>
//RTN inline VirtualCall(void* _this, uintptr_t off, Args... args) {
//	return (*(RTN(**)(void*, Args...))(*(uintptr_t*)_this + off))(_this, args...);
//}


vector<string> loadAllPlayerIDs();
