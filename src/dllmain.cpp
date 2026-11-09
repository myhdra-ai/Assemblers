// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Hooks.h"

#include <atomic>

namespace
{
    std::atomic_bool g_running = true;

    DWORD WINAPI InitializeThread(LPVOID module)
    {
        while (g_running && !Hooks::Initialize())
            Sleep(1000);

        while (g_running)
            Sleep(100);

        Hooks::Shutdown();
        FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
        return 0;
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitializeThread, hModule, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_running = false;
        break;
    }
    return TRUE;
}

