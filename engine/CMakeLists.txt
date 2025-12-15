cmake_minimum_required(VERSION 3.15)
project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find Lua
# On macOS with Homebrew, help CMake find Lua
if(APPLE)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/opt/homebrew;/usr/local")
    set(LUA_INCLUDE_DIR "/opt/homebrew/include/lua5.4" CACHE PATH "Lua include directory")
    set(LUA_LIBRARY "/opt/homebrew/lib/liblua.dylib" CACHE FILEPATH "Lua library")
endif()

find_package(Lua 5.4 REQUIRED)
if(LUA_FOUND)
    message(STATUS "Found Lua: ${LUA_VERSION_STRING}")
    message(STATUS "Lua include dir: ${LUA_INCLUDE_DIR}")
    message(STATUS "Lua libraries: ${LUA_LIBRARIES}")
else()
    message(FATAL_ERROR "Lua not found! Please install Lua 5.4+")
endif()

# Find Sol3 (header-only)
set(SOL3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/sol3/include)
if(EXISTS "${SOL3_INCLUDE_DIR}/sol/sol.hpp")
    message(STATUS "Found Sol3: ${SOL3_INCLUDE_DIR}/sol/sol.hpp")
    include_directories(${SOL3_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Sol3 not found! Please run: ./scripts/download_sol3.sh")
endif()

# ECS Library
add_library(ecs STATIC
    src/ecs/EntityManager.cpp
    src/ecs/ComponentManager.cpp
    src/ecs/SystemManager.cpp
    src/ecs/Coordinator.cpp
)

target_include_directories(ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Network library (header-only for serialization)
add_library(network INTERFACE)

target_include_directories(network INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link network to ECS for Types.hpp
target_link_libraries(network INTERFACE ecs)

# Rendering library
add_library(rendering STATIC
    src/rendering/Window.cpp
    src/rendering/Camera.cpp
)

target_include_directories(rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: Build examples
option(BUILD_ENGINE_EXAMPLES "Build engine examples" ON)

if(BUILD_ENGINE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Core library (for SystemLoader)
add_library(core STATIC
    src/core/SystemLoader.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC ecs ${CMAKE_DL_LIBS})

# Scripting library (Lua + Sol3)
add_library(scripting STATIC
    src/scripting/LuaState.cpp
)

target_include_directories(scripting PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
    ${SOL3_INCLUDE_DIR}
)

target_link_libraries(scripting PUBLIC
    ecs
    ${LUA_LIBRARIES}
)

# Build system libraries
add_subdirectory(systems)

# Installation
install(TARGETS ecs rendering network core scripting
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
