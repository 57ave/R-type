cmake_minimum_required(VERSION 3.15)
project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options de build
option(BUILD_CLIENT "Build client executable" ON)
option(BUILD_SERVER "Build server executable" ON)
option(BUILD_GAME "Build game executable" ON)
option(BUILD_TESTS "Build tests" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find SFML (required by all sub-projects)
find_package(SFML 2.5 COMPONENTS system window graphics audio network REQUIRED)

# Build engine library
add_subdirectory(engine)

# Build client if requested
if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

# Build server if requested
if(BUILD_SERVER)
    add_subdirectory(server)
endif()

# Build game if requested
if(BUILD_GAME)
    add_subdirectory(game)
endif()
