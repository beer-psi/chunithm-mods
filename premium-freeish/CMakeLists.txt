cmake_minimum_required(VERSION 3.27)
project(chunithm_pfreeish VERSION 1.6.0 LANGUAGES CXX)

file(GLOB_RECURSE VERSIONS "versions/*.cmake")
file(READ cmake/VersionOffsets.in VERSION_OFFSET_TEMPLATE)

foreach (VERSION_PATH ${VERSIONS})
  message(STATUS "Including game support file: ${VERSION_PATH}")

  include(${VERSION_PATH})

  string(CONFIGURE ${VERSION_OFFSET_TEMPLATE} VERSION_OFFSET_TEXT @ONLY)
  string(APPEND VERSION_ALL_OFFSETS "${VERSION_OFFSET_TEXT}")
endforeach ()

configure_file(cmake/VersionHeader.in macros/versions.h @ONLY)

add_library(${PROJECT_NAME} SHARED src/dllmain.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE safetyhook)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/macros ../vendor)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "META_PROJECT_VERSION=\"${PROJECT_VERSION}\""
)
