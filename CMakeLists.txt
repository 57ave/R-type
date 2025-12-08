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
option(BUILD_TESTS "Build tests" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find SFML (needed for client)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Subdirectories
add_subdirectory(engine)
add_subdirectory(game)

if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(BUILD_SERVER)
    add_subdirectory(server)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
