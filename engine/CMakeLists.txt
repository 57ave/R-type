cmake_minimum_required(VERSION 3.15)
project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX C OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings (only for our code, not dependencies)
if(MSVC)
    add_compile_options(/W4)
else()
    # Use generator expressions to apply warnings only to CXX
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall> $<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
    # Remove -Werror for Objective-C files (SFML macOS issue)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-error>)
    add_compile_options($<$<COMPILE_LANGUAGE:OBJC>:-Wno-error>)
    add_compile_options($<$<COMPILE_LANGUAGE:OBJCXX>:-Wno-error>)
endif()

# Find Lua (optional for scripting)
find_package(Lua)

# Sol3 header-only library
set(SOL3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sol3/include" CACHE PATH "Path to Sol3 include directory")
if(NOT EXISTS "${SOL3_INCLUDE_DIR}/sol/sol.hpp")
    message(WARNING "Sol3 not found at ${SOL3_INCLUDE_DIR}. Scripting features will be disabled.")
    set(LUA_FOUND FALSE)
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

# Inclure CPM.cmake pour la gestion des dépendances
include(cmake/CPM.cmake)

# Dépendance ASIO
CPMAddPackage("gh:chriskohlhoff/asio#asio-1-28-0")

# Dépendance SFML: Force SFML 2.6 via CPM (project not compatible with SFML 3.x API)
# Skip system SFML to avoid version conflicts
option(USE_SYSTEM_SFML "Use system SFML instead of CPM" OFF)

# SFML build options - disable warnings as errors (causes issues on macOS with ObjC files)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

if(USE_SYSTEM_SFML)
    find_package(SFML 2.6 COMPONENTS graphics window system audio QUIET)
endif()
if (NOT SFML_FOUND)
    message(STATUS "Using CPM to download SFML 2.6.0")
    # Add SFML with OPTIONS to disable warnings as errors
    CPMAddPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 2.6.0
        OPTIONS
            "WARNINGS_AS_ERRORS OFF"
            "SFML_BUILD_EXAMPLES OFF"
    )
endif()
set(SFML_VERSION_MAJOR 2)

# Enable PIC for ecs library (needed for shared library systems)
set_target_properties(ecs PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Network library
add_library(network STATIC
    src/network/UdpServer.cpp
    src/network/NetworkServer.cpp
    src/network/UdpClient.cpp
    src/network/NetworkClient.cpp
)

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${asio_SOURCE_DIR}/asio/include
)

target_link_libraries(network PUBLIC ecs)

set_target_properties(network PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Core library (for SystemLoader and engine abstractions)
# Must be defined before rendering because rendering depends on it
add_library(core STATIC
    src/core/SystemLoader.cpp
    src/core/Logger.cpp
    src/core/Profiler.cpp
    src/core/ProfilerOverlay.cpp
    src/core/DevConsole.cpp
    src/engine/Input.cpp
    src/engine/Keyboard.cpp
    src/engine/Clock.cpp
    src/engine/Audio.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC ecs sfml-graphics sfml-window sfml-system sfml-audio ${CMAKE_DL_LIBS})

set_target_properties(core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Rendering library
add_library(rendering STATIC
    src/rendering/Camera.cpp
    src/rendering/sfml/SFMLRenderer.cpp
    src/rendering/sfml/SFMLSprite.cpp
    src/rendering/sfml/SFMLTexture.cpp
    src/rendering/sfml/SFMLWindow.cpp
    src/systems/RenderSystem.cpp
)

target_include_directories(rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rendering PUBLIC
    ecs
    core
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

set_target_properties(rendering PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Main engine library (combines all components)
add_library(engine INTERFACE)

target_link_libraries(engine INTERFACE
    ecs
    rendering
    core
    scripting
    network
)

# Optional: Build examples
option(BUILD_ENGINE_EXAMPLES "Build engine examples" OFF)

if(BUILD_ENGINE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Scripting library (Lua + Sol3) - only if Lua is found
if(LUA_FOUND)
    add_library(scripting STATIC
        src/scripting/LuaState.cpp
        src/scripting/ComponentBindings.cpp
        src/scripting/DevConsoleBindings.cpp
        src/scripting/GameBindings.cpp
        src/scripting/PrefabManager.cpp
        src/scripting/ScriptSystem.cpp
        src/scripting/ScriptingManager.cpp
    )

    target_include_directories(scripting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LUA_INCLUDE_DIR}
        ${SOL3_INCLUDE_DIR}
    )

    target_link_libraries(scripting PUBLIC
        ecs
        core
        ${LUA_LIBRARIES}
    )
    
    target_compile_definitions(scripting PUBLIC RTYPE_SCRIPTING_ENABLED=1)
else()
    message(STATUS "Lua not found. Scripting library will not be built.")
    # Create an INTERFACE library as placeholder
    add_library(scripting INTERFACE)
    target_compile_definitions(scripting INTERFACE RTYPE_SCRIPTING_ENABLED=0)
endif()

# System libraries
# Movement System
add_library(MovementSystem SHARED
    src/systems/MovementSystem.cpp
)

target_include_directories(MovementSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MovementSystem PUBLIC ecs)

# Collision System
add_library(CollisionSystem SHARED
    src/systems/CollisionSystem.cpp
)

target_include_directories(CollisionSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(CollisionSystem PUBLIC ecs)

# Input System
add_library(InputSystem SHARED
    src/systems/InputSystem.cpp
)

target_include_directories(InputSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(InputSystem PUBLIC ecs)

# Lifetime System
add_library(LifetimeSystem SHARED
    src/systems/LifetimeSystem.cpp
)

target_include_directories(LifetimeSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LifetimeSystem PUBLIC ecs)

# Animation System
add_library(AnimationSystem SHARED
    src/systems/AnimationSystem.cpp
)

target_include_directories(AnimationSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AnimationSystem PUBLIC ecs rendering)

# StateMachine Animation System (for player ship animations)
add_library(StateMachineAnimationSystem SHARED
    src/systems/StateMachineAnimationSystem.cpp
)

target_include_directories(StateMachineAnimationSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(StateMachineAnimationSystem PUBLIC ecs rendering)

# Weapon System
add_library(WeaponSystem SHARED
    src/systems/WeaponSystem.cpp
)

target_include_directories(WeaponSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(WeaponSystem PUBLIC ecs)

# Enemy Spawn System
add_library(EnemySpawnSystem SHARED
    src/systems/EnemySpawnSystem.cpp
)

target_include_directories(EnemySpawnSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(EnemySpawnSystem PUBLIC ecs)

# Movement Pattern System
add_library(MovementPatternSystem SHARED
    src/systems/MovementPatternSystem.cpp
)

target_include_directories(MovementPatternSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MovementPatternSystem PUBLIC ecs)

# Scrolling Background System
add_library(ScrollingBackgroundSystem SHARED
    src/systems/ScrollingBackgroundSystem.cpp
)

target_include_directories(ScrollingBackgroundSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ScrollingBackgroundSystem PUBLIC ecs rendering)

# Boundary System
add_library(BoundarySystem SHARED
    src/systems/BoundarySystem.cpp
)

target_include_directories(BoundarySystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BoundarySystem PUBLIC ecs)

# Health System
add_library(HealthSystem SHARED
    src/systems/HealthSystem.cpp
)

target_include_directories(HealthSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(HealthSystem PUBLIC ecs)

# Installation
install(TARGETS ecs rendering core scripting network
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
