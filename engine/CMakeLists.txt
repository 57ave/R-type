cmake_minimum_required(VERSION 3.15)

# Conditionally enable Objective-C/C++ only on macOS
if(APPLE)
    project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX C OBJC OBJCXX)
else()
    project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX C)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings (only for our code, not dependencies)
if(MSVC)
    add_compile_options(/W4)
else()
    # Use generator expressions to apply warnings only to CXX
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall> $<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
    # Remove -Werror for C files
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-error>)
    # Only add Objective-C flags on macOS
    if(APPLE)
        add_compile_options($<$<COMPILE_LANGUAGE:OBJC>:-Wno-error>)
        add_compile_options($<$<COMPILE_LANGUAGE:OBJCXX>:-Wno-error>)
    endif()
endif()

# ========================================
# LUA/SOL3 CONFIGURATION
# ========================================
option(ENABLE_SCRIPTING "Enable Lua/Sol3 scripting support" ON)

if(ENABLE_SCRIPTING)
    if(NOT CMAKE_CROSSCOMPILING)
        find_package(Lua 5.4 QUIET)
        if(LUA_FOUND)
            set(LUA_LIBRARIES ${LUA_LIBRARIES})
            set(LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR})
            set(LUA_FOUND TRUE)
        endif()
    endif()

    if(NOT LUA_FOUND)
        message(STATUS "Lua 5.4 not found on system (or cross-compiling). Fetching via CPM...")
        
        CPMAddPackage(
            NAME Lua
            GITHUB_REPOSITORY "lua/lua"
            GIT_TAG "v5.4.6"
            DOWNLOAD_ONLY YES
        )
        
        if(Lua_ADDED OR EXISTS "${Lua_SOURCE_DIR}")
            message(STATUS "Configuring Lua manually from source...")
            file(GLOB LUA_SOURCES "${Lua_SOURCE_DIR}/*.c")
            list(REMOVE_ITEM LUA_SOURCES "${Lua_SOURCE_DIR}/lua.c" "${Lua_SOURCE_DIR}/luac.c" "${Lua_SOURCE_DIR}/onelua.c")

            if(WIN32 OR MINGW)
                add_library(lua SHARED ${LUA_SOURCES})
                target_compile_definitions(lua PUBLIC LUA_BUILD_AS_DLL)
                target_compile_definitions(lua PRIVATE RTYPE_BUILD_DLL)
            else()
                add_library(lua STATIC ${LUA_SOURCES})
                target_compile_definitions(lua PUBLIC LUA_USE_LINUX)
            endif()
            target_include_directories(lua PUBLIC ${Lua_SOURCE_DIR})
            set_target_properties(lua PROPERTIES POSITION_INDEPENDENT_CODE ON)

            set(LUA_LIBRARIES lua)
            set(LUA_INCLUDE_DIR ${Lua_SOURCE_DIR})
            set(LUA_FOUND TRUE)
        endif()
    endif()

    if(LUA_FOUND)
        CPMAddPackage(
            NAME sol2
            GITHUB_REPOSITORY "ThePhD/sol2"
            GIT_TAG "v3.3.1"
        )
        
        if(sol2_ADDED)
            set(SOL3_INCLUDE_DIR ${sol2_SOURCE_DIR}/include CACHE PATH "" FORCE)
            message(STATUS "Sol2/Sol3 fetched via CPM")
            
            # -Wtemplate-body only exists in GCC 15+ and Clang — skip on older GCC
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15")
                add_compile_options(-Wno-template-body)
            elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                add_compile_options(-Wno-template-body)
            endif()
        else()
             if(NOT DEFINED SOL3_INCLUDE_DIR)
                 set(SOL3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sol3/include" CACHE PATH "" FORCE)
             endif()
        endif()

        message(STATUS " Scripting ENABLED with Lua and Sol3")
    else()
        message(WARNING " Lua not found and could not be fetched. Scripting features will be disabled.")
        set(ENABLE_SCRIPTING OFF)
    endif()
else()
    message(STATUS "ℹ Scripting disabled by user option")
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

if(WIN32)
    target_link_libraries(network PUBLIC ws2_32 wsock32)
endif()

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
    src/rendering/sfml/SFMLFont.cpp
    src/rendering/sfml/SFMLText.cpp
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
        src/scripting/CoreBindings.cpp
        src/scripting/DevConsoleBindings.cpp
        src/scripting/GameBindings.cpp
        src/scripting/PrefabManager.cpp
        src/scripting/ScriptSystem.cpp
        src/scripting/ScriptingManager.cpp
    )
    set_target_properties(scripting PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_include_directories(scripting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LUA_INCLUDE_DIR}
        ${SOL3_INCLUDE_DIR}
    )

    # Link Lua library - use the detected library from architecture-aware detection
    target_link_libraries(scripting PUBLIC ecs core rendering ${LUA_LIBRARIES})
    
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
target_compile_definitions(MovementSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(MovementSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(MovementSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MovementSystem PUBLIC ecs core)

# Collision System - MOVED TO GAME (R-Type specific)
# add_library(CollisionSystem SHARED
#     src/systems/CollisionSystem.cpp
# )
# 
# target_include_directories(CollisionSystem PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
# 
# target_link_libraries(CollisionSystem PUBLIC ecs)

# Input System
add_library(InputSystem SHARED
    src/systems/InputSystem.cpp
)
target_compile_definitions(InputSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(InputSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(InputSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(InputSystem PUBLIC ecs core)

# Lifetime System
add_library(LifetimeSystem SHARED
    src/systems/LifetimeSystem.cpp
)
target_compile_definitions(LifetimeSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(LifetimeSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(LifetimeSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LifetimeSystem PUBLIC ecs core)

# Animation System
add_library(AnimationSystem SHARED
    src/systems/AnimationSystem.cpp
)
target_compile_definitions(AnimationSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(AnimationSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(AnimationSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AnimationSystem PUBLIC ecs rendering core)

# StateMachine Animation System (for player ship animations)
add_library(StateMachineAnimationSystem SHARED
    src/systems/StateMachineAnimationSystem.cpp
)
target_compile_definitions(StateMachineAnimationSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(StateMachineAnimationSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(StateMachineAnimationSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(StateMachineAnimationSystem PUBLIC ecs rendering core)

# NOTE: Weapon, Enemy Spawn, and Movement Pattern systems moved to shootemup module
# See engine/modules/shootemup/

# Scrolling Background System
add_library(ScrollingBackgroundSystem SHARED
    src/systems/ScrollingBackgroundSystem.cpp
)
target_compile_definitions(ScrollingBackgroundSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(ScrollingBackgroundSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(ScrollingBackgroundSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ScrollingBackgroundSystem PUBLIC ecs rendering core)

# Boundary System
add_library(BoundarySystem SHARED
    src/systems/BoundarySystem.cpp
)
target_compile_definitions(BoundarySystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(BoundarySystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(BoundarySystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BoundarySystem PUBLIC ecs)

# Health System
add_library(HealthSystem SHARED
    src/systems/HealthSystem.cpp
)
target_compile_definitions(HealthSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(HealthSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(HealthSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(HealthSystem PUBLIC ecs)

# UI System - for menus and UI elements (only if Lua/scripting is available)
if(LUA_FOUND)
    add_library(UISystem SHARED
        src/systems/UISystem.cpp
        src/systems/UISystemRender.cpp
        src/systems/UISystemEntities.cpp
    )
    target_compile_definitions(UISystem PRIVATE RTYPE_BUILD_DLL)
    if(WIN32 OR MINGW)
        target_link_options(UISystem PRIVATE -Wl,--export-all-symbols)
    endif()

    target_include_directories(UISystem PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LUA_INCLUDE_DIR}
        ${SOL3_INCLUDE_DIR}
    )

    # Link Lua library - use the detected library from architecture-aware detection
    target_link_libraries(UISystem PUBLIC ecs rendering scripting ${LUA_LIBRARIES} core)

    # Add to engine interface
    target_link_libraries(engine INTERFACE UISystem)
else()
    message(STATUS "UISystem disabled - requires Lua/Sol3")
endif()

# Audio System - for sound effects and music management
add_library(AudioSystem SHARED
    src/systems/AudioSystem.cpp
)
target_compile_definitions(AudioSystem PRIVATE RTYPE_BUILD_DLL)
if(WIN32 OR MINGW)
    target_link_options(AudioSystem PRIVATE "-Wl,--exclude-libs,ALL")
endif()

target_include_directories(AudioSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AudioSystem PUBLIC ecs core)

# Add to engine interface
target_link_libraries(engine INTERFACE AudioSystem)

# Installation
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS ecs rendering core scripting network
    MovementSystem
    InputSystem
    LifetimeSystem
    AnimationSystem
    StateMachineAnimationSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    AudioSystem
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

if(TARGET UISystem)
    install(TARGETS UISystem
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

if(TARGET lua)
    install(TARGETS lua
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# ========================================
# OPTIONAL MODULES
# ========================================

# Shoot'em Up Module (optional, can be disabled)
option(BUILD_SHOOTEMUP_MODULE "Build the Shoot'em Up game module" ON)

if(BUILD_SHOOTEMUP_MODULE)
    message(STATUS " Building Shoot'em Up module")
    add_subdirectory(modules/shootemup)
endif()
