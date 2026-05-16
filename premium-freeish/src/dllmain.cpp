//
// Created by beerpsi on 4/17/24.
//

#include <windows.h>
#include <psapi.h>

#include "safetyhook.hpp"
#include "ppp.h"

#include "offsets.h"
#include "versions.h"

// ref: App/data/db/ModeTableRecord.bin
enum GameMode : int {
    Normal = 0,
    Course = 1,
    NationalMatching = 2,
    UnlockChallenge = 3,
    LinkedVerse = 4,
};

auto static chusan_app = PBYTE {};
auto static offsets = offsets_t {};

auto static hook_track_count = SafetyHookInline {};
auto static hook_end_of_track = SafetyHookMid {};

auto static vk_end_credit = VK_ESCAPE;
auto static max_tracks = -1;

// SDGS 1.10-1.15 / SDHD 2.00-2.20
constexpr auto find_hook_track_count_1 = &ppp::any<"8D ?? 78 E8 r[?? ?? ?? ??] 8D ?? 78 8B F0"_pattern>;
// SDGS 1.30/SDHD 2.26-2.45
constexpr auto find_hook_track_count_2 = &ppp::any<"8D ?? ?? 8B ?? E8 r[?? ?? ?? ??] 8B ?? 8B F0 E8"_pattern>;

constexpr auto find_hook_end_of_track = &ppp::any<"o[E8] ?? ?? ?? ?? 3C 01 75 44 8B CF E8"_pattern>;

void dprintf(const wchar_t *fmt, ...) {
    static wchar_t buf[1024];
    va_list ap;

    va_start(ap, fmt);
    vswprintf_s(buf, 1024, fmt, ap);
    va_end(ap);

    OutputDebugStringW(buf);
#ifndef NDEBUG
    printf("%ls", buf);
#endif
    buf[0] = L'\0';
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_DETACH) {
        dprintf(L"PremiumFreeish: Disabling\n");

        hook_track_count.reset();
        hook_end_of_track.reset();
        return TRUE;
    }

    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    dprintf(L"PremiumFreeish v" META_PROJECT_VERSION ": Loaded at 0x%p\n", hinstDLL);

    chusan_app = reinterpret_cast<PBYTE>(GetModuleHandleA(nullptr));

    if (!chusan_app) {
        return FALSE;
    }

    auto const dos = reinterpret_cast<PIMAGE_DOS_HEADER>(chusan_app);
    auto const nt = reinterpret_cast<PIMAGE_NT_HEADERS>(chusan_app + dos->e_lfanew);
    auto const it = std::ranges::find_if(versions, [&](const offsets_t& version) {
        return version.time_date_stamp == nt->FileHeader.TimeDateStamp
                && version.address_of_entry_point == nt->OptionalHeader.AddressOfEntryPoint;
    });

    if (it == versions.end()) {
        dprintf(L"PremiumFreeish: Could not find predefined offsets, trying signature scanning...\n");

        MODULEINFO modinfo;
        HRESULT result = GetModuleInformation(
            GetCurrentProcess(),
            GetModuleHandleA(nullptr),
            &modinfo,
            sizeof(MODULEINFO));

        if (result == 0) {
            dprintf(L"PremiumFreeish: Could not get module information, error code %#08lx\n", GetLastError());
            return TRUE;
        }

        const auto base_address = reinterpret_cast<uint64_t>(modinfo.lpBaseOfDll);
        const auto memory_span = std::span<char const>(reinterpret_cast<char *>(base_address), static_cast<size_t>(modinfo.SizeOfImage));

        if (auto const track_count_match_1 = find_hook_track_count_1(memory_span, 0)) {
            offsets.hook_track_count = std::get<1>(*track_count_match_1);
        } else if (auto const track_count_match_2 = find_hook_track_count_2(memory_span, 0)) {
            offsets.hook_track_count = std::get<1>(*track_count_match_2);
        } else {
            dprintf(L"PremiumFreeish: Failed to find offset for hook_track_count\n");
            return TRUE;
        }

        auto const end_of_track_match = find_hook_end_of_track(memory_span, 0);

        if (!end_of_track_match) {
            dprintf(L"PremiumFreeish: Failed to find offset for hook_end_of_track\n");
            return TRUE;
        }

        offsets.hook_end_of_track = std::get<1>(*end_of_track_match);

        dprintf(
            L"PremiumFreeish: TimeDateStamp=%#08lx, AddressOfEntryPoint=%#08lx, hook_track_count=%#08lx, hook_end_of_track=%#08lx\n",
            nt->FileHeader.TimeDateStamp,
            nt->OptionalHeader.AddressOfEntryPoint,
            offsets.hook_track_count,
            offsets.hook_end_of_track);
    } else {
        dprintf(L"PremiumFreeish: Found predefined addresses\n");
        offsets = *it;
    }

    vk_end_credit = GetPrivateProfileIntW(
        L"pfreeish",
        L"end_credit",
        VK_ESCAPE,
        L".\\segatools.ini");
    max_tracks = GetPrivateProfileIntW(
        L"pfreeish",
        L"max_tracks",
        -1,
        L".\\segatools.ini");

    hook_track_count = safetyhook::create_inline(
        chusan_app + offsets.hook_track_count,
        +[] {
            // Return the correct number of tracks so that tower LEDs look nice
            return max_tracks > 0 ? max_tracks : 1;
        });

    // The game calls a function to check whether the credit should be ended, and we
    // overwrite that call with a check of whether we're holding the escape key or not.
    // This is probably safer than relying on arbitrary offsets, and we don't have to do
    // any hardcoding of instructions because a function call is also 5 bytes.
    hook_end_of_track = safetyhook::create_mid(
        chusan_app + offsets.hook_end_of_track,
        [] (safetyhook::Context& ctx) {
            // The original function is __thiscall, so the `this` pointer is on ecx
            // Knowing this, we can get the game mode from the `this` pointer.
            const auto track_state_ptr = *reinterpret_cast<int *>(ctx.ecx + 4);

            // ReSharper disable once CppTooWideScopeInitStatement
            const int game_mode = *reinterpret_cast<int *>(track_state_ptr + 44);

            // run our shenanigans only in standard/unlock challenge mode
            // don't do anything for national matching/course mode, let the game deal with it
            if (game_mode != static_cast<int>(GameMode::Normal)
                && game_mode != static_cast<int>(GameMode::UnlockChallenge)
                && game_mode != static_cast<int>(GameMode::LinkedVerse)) {
                return;
            }

            // By default, SafetyHookMid will call the original code that it's replacing.
            // We don't really want that; so we just skip over it by modifying the instruction
            // pointer.
            // However, we *do* want the original function to be run for special modes, so the game
            // can handle end-of-track properly for these cases.
            ctx.eip = reinterpret_cast<uintptr_t>(chusan_app + offsets.hook_end_of_track + 5);

            // If the user has a positive number of max tracks set, and the current track
            // exceeds that, then end the credit. This is for users who wish to use the
            // "hold escape" feature to exit early, but doesn't want infinite tracks.
            //
            // We handle this instead of handing off to the game because the game has
            // a hard cap of 7 tracks. There are patches to work around that, but I'd
            // rather not have to scan/store offsets for another patch.
            if (max_tracks > 0) {
                int current_track = *reinterpret_cast<int *>(track_state_ptr + 32);

                if (current_track >= max_tracks) {
                    ctx.eax = 1;
                    return;
                }
            }

            if ((GetAsyncKeyState(vk_end_credit) & 0x8000) == 0) {
                ctx.eax = 0;
                return;
            }

            ctx.eax = 1;
        });

    dprintf(L"PremiumFreeish: Enabled\n");

    return TRUE;
}
