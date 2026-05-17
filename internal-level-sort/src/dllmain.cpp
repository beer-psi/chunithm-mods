#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#include <safetyhook.hpp>

#include "ppp.h"

struct MusicEntry {
    int32_t music_id;
    uint8_t difficulty;
};

enum PlayOptionSortMusicFilterLvID : int32_t {
    MUSIC_LEVEL = 2,
    MUSIC_LEVEL_DOWN = 10,
};

static uintptr_t chusan_app = {};

auto static hook_music_level_comparator = SafetyHookInline {};
int32_t(*get_chart_internal_level)(int32_t musicId, uint8_t* difficulty) = nullptr;
PlayOptionSortMusicFilterLvID* sort_music_filter_lv_id = (PlayOptionSortMusicFilterLvID*)nullptr;

constexpr auto find_music_level_comparator = &ppp::any<"o[83] EC 08 8D 44 24 0C 53"_pattern>;
constexpr auto find_get_chart_internal_level_fn = &ppp::any<"o[51] E8 ? ? ? ? 3C 01 0F 85 ? ? ? ? 55"_pattern>;
constexpr auto find_sort_music_filter_lv_id_address = &ppp::any<"83 3D u[?? ?? ?? ??] 02 75 ??"_pattern>;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_DETACH) {
        printf("InternalLevelSort: Disabling\n");
        hook_music_level_comparator.reset();
        return TRUE;
    }

    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(hinstDLL);

    chusan_app = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));

    MODULEINFO modinfo;
    HRESULT result = GetModuleInformation(
        GetCurrentProcess(),
        GetModuleHandleA(nullptr),
        &modinfo,
        sizeof(MODULEINFO)
    );

    if (result == 0) {
        printf("InternalLevelSort: Could not get module information, error code %#08lx\n", GetLastError());
        return FALSE;
    }

    const auto memory_span = std::span<char const>(reinterpret_cast<char *>(modinfo.lpBaseOfDll), static_cast<size_t>(modinfo.SizeOfImage));
    const auto music_level_comparator_match = find_music_level_comparator(memory_span, (uint64_t)modinfo.lpBaseOfDll);
    const auto get_chart_internal_level_match = find_get_chart_internal_level_fn(memory_span, (uint64_t)modinfo.lpBaseOfDll);

    if (!music_level_comparator_match || !get_chart_internal_level_match) {
        printf("InternalLevelSort: Could not find address of level sort function or get internal level function\n");
        return FALSE;
    }

    uintptr_t music_level_comparator_addr = std::get<1>(*music_level_comparator_match);
    get_chart_internal_level = reinterpret_cast<decltype(get_chart_internal_level)>(std::get<1>(*get_chart_internal_level_match));

    const auto music_level_comparator_span = std::span<char const>(reinterpret_cast<char *>(music_level_comparator_addr), 500);
    const auto sort_music_filter_lv_id_match = find_sort_music_filter_lv_id_address(music_level_comparator_span, music_level_comparator_addr);

    if (!sort_music_filter_lv_id_match) {
        printf("InternalLevelSort: Could not find address of music sort type variable\n");
        return FALSE;
    }

    sort_music_filter_lv_id = reinterpret_cast<decltype(sort_music_filter_lv_id)>(std::get<1>(*sort_music_filter_lv_id_match));
    hook_music_level_comparator = safetyhook::create_inline(
        music_level_comparator_addr,
        +[] (struct MusicEntry* a, struct MusicEntry* b) {
            uint8_t difficulty = a->difficulty;
            int internalLevelA = get_chart_internal_level(a->music_id, &difficulty);

            difficulty = b->difficulty;
            int32_t internalLevelB = get_chart_internal_level(b->music_id, &difficulty);

            if (internalLevelA == internalLevelB) {
                return hook_music_level_comparator.call<bool>(a, b);
            }

            return *sort_music_filter_lv_id == PlayOptionSortMusicFilterLvID::MUSIC_LEVEL
                ? internalLevelA < internalLevelB
                : internalLevelA > internalLevelB;
        }
    );

    printf("InternalLevelSort: Enabled\n");

    return TRUE;
}
