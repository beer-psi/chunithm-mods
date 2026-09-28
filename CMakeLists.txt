cmake_minimum_required(VERSION 3.27)
project(chunithm_mods VERSION 0.1.0 LANGUAGES CXX)

include(CheckIPOSupported)
include(FetchContent)

option(STATIC_MSVC_RUNTIME "Static link MSVC runtime" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SYSTEM_PROCESSOR "i386")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
if (STATIC_MSVC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(SAFETYHOOK_FETCH_ZYDIS ON)
FetchContent_Declare(
    safetyhook
    GIT_REPOSITORY "https://github.com/cursey/safetyhook.git"
    GIT_TAG "c3f3f306a0f12d1811c0b713ad2ed2a8ddc6cf55" # v0.6.9
)
FetchContent_MakeAvailable(safetyhook)

check_ipo_supported(RESULT IPO_SUPPORTED)
if (IPO_SUPPORTED)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

add_subdirectory(internal-level-sort)
add_subdirectory(credit-clock)
add_subdirectory(premium-freeish)
