set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Windows)

if(DEFINED ENV{INCLUDE} AND NOT DEFINED _MSVCENV_INCLUDE_DIRS)
    set(
        _MSVCENV_INCLUDE_DIRS
        "$ENV{INCLUDE}"
        CACHE STRING
        "Include directories for MSVC environment"
    )
elseif(NOT DEFINED _MSVCENV_INCLUDE_DIRS)
    message(FATAL_ERROR "INCLUDE environment variable not set")
endif()

foreach(dir IN LISTS _MSVCENV_INCLUDE_DIRS)
    string(REPLACE "\\" "/" dir "${dir}")

    if(dir MATCHES "^z:/")
        string(REGEX REPLACE "^z:" "" dir "${dir}")
    endif()

    include_directories(SYSTEM "${dir}")
endforeach()

if(DEFINED ENV{LIB} AND NOT DEFINED _MSVCENV_LIB_DIRS)
    set(
        _MSVCENV_LIB_DIRS
        "$ENV{LIB}"
        CACHE STRING
        "Library directories for MSVC environment"
    )
elseif(NOT DEFINED _MSVCENV_LIB_DIRS)
    message(FATAL_ERROR "LIB environment variable not set")
endif()

foreach(dir IN LISTS _MSVCENV_LIB_DIRS)
    string(REPLACE "\\" "/" dir "${dir}")

    if(dir MATCHES "^z:")
        string(REGEX REPLACE "^z:" "" dir "${dir}")
    endif()

    link_directories("${dir}")
endforeach()
