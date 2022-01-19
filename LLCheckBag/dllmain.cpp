// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#pragma comment(lib, "../LiteLoaderSDK/Lib/bedrock_server_api.lib")
#pragma comment(lib, "../LiteLoaderSDK/Lib/bedrock_server_var.lib")
#pragma comment(lib, "../LiteLoaderSDK/Lib/SymDBHelper.lib")
#pragma comment(lib, "../LiteLoaderSDK/Lib/LiteLoader.lib")
#include <HookAPI.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LL::registerPlugin("LLCheckBag", "Inventory check plugin that depends on LiteLoader", 
            LL::Version(
                PLUGIN_VERSION_MAJOR,
                PLUGIN_VERSION_MINOR,
                PLUGIN_VERSION_REVISION,
                PLUGIN_VERSION_IS_BETA ? LL::Version::Beta : LL::Version::Release
            ), {
                    { "Git", "https://github.com/quizhizhe/LLCheckBag" },
                    //{ "License", PLUGIN_LICENCE },
                    //{ "Website", PLUGIN_WEBSIDE },
            });
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
};

void PluginInit();

extern "C" {
    _declspec(dllexport) void onPostInit() {
        std::ios::sync_with_stdio(false);
        PluginInit();
    }
}
