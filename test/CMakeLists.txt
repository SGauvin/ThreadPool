cmake_minimum_required(VERSION 3.12)

add_compile_definitions(TURBO_ENABLE_CLIENT_LOG)
add_compile_definitions(TURBO_ENABLE_ENGINE_LOG)
add_compile_definitions(TURBO_ENABLE_ASSERT)

project(test)
add_executable(${CMAKE_PROJECT_NAME} test.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ../)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_23)
