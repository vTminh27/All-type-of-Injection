#include <windows.h>
#include "pch.h"

extern "C" __declspec(dllexport) void HelloWorld() {
    MessageBox(NULL,L"Malicious DLL Loaded!",L"DLL Sideloading", MB_OK);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL,L"Malicious DLL Attached!",L"DLL Sideloading", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}