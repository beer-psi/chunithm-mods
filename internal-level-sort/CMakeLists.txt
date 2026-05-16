cmake_minimum_required(VERSION 3.27)
project(chunithm_internal_level_sort VERSION 0.1.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} SHARED src/dllmain.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE safetyhook)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/macros ../vendor)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "META_PROJECT_VERSION=\"${PROJECT_VERSION}\""
)
