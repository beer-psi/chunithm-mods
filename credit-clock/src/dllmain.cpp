#include <cstdint>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#include <safetyhook.hpp>

#include "ppp.h"

// if this signature breaks, just search for the string "FREE PLAY" and go to the only
// function refering to the string
constexpr auto findCreditTextFunction = &ppp::any<"o[81] EC ?? ?? ?? ?? A1 ?? ?? ?? ?? 33 C4 89 84 24 ?? ?? ?? ?? 83 79"_pattern>;
SafetyHookInline hookCreditTextFunction {};

struct std_string {
    union {
        char data[16];
        char* ptr;
    };
    uint32_t length;
    uint32_t capacity;
};

decltype(GetLocalTime)* g_realGetLocalTime = NULL;

void* __fastcall creditTextFunctionDetour(void* self, void* edx, void* a2) {
    void* result = hookCreditTextFunction.fastcall<void*>(self, edx, a2);

    SYSTEMTIME now = { 0 };
    g_realGetLocalTime(&now);

    std_string* creditText = (std_string*)((uintptr_t)self + 0x4C);
    char* buffer = creditText->capacity > 15 ? creditText->ptr : creditText->data;

    // note that since our format string is always smaller than the smallest string
    // (15 characters), we skip the check capacity + allocate step. if you need to
    // format a bigger string you will need to take care of this.

    creditText->length = snprintf(
        buffer,
        creditText->capacity + 1,
        "%02u:%02u:%02u",
        now.wHour, now.wMinute, now.wSecond
    );

    return result;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_DETACH) {
        hookCreditTextFunction.reset();
        return TRUE;
    }

    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(hinstDLL);

    g_realGetLocalTime = (decltype(g_realGetLocalTime))GetProcAddress(LoadLibraryA("kernel32.dll"), "GetLocalTime");

    if (g_realGetLocalTime == NULL) {
        return FALSE;
    }

    MODULEINFO moduleInfo;
    HRESULT result = GetModuleInformation(
        GetCurrentProcess(),
        GetModuleHandleA(nullptr),
        &moduleInfo,
        sizeof(MODULEINFO)
    );

    if (result == 0) {
        // todo logging
        return FALSE;
    }

    const auto moduleSpan = std::span<char const>((char*)moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage);
    const auto creditTextFunctionMatch = findCreditTextFunction(moduleSpan, (uintptr_t)moduleInfo.lpBaseOfDll);

    if (!creditTextFunctionMatch) {
        return FALSE;
    }

    hookCreditTextFunction = safetyhook::create_inline(std::get<1>(*creditTextFunctionMatch), creditTextFunctionDetour);

    return TRUE;
}
