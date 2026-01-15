cmake_minimum_required(VERSION 3.15)
project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX C OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
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

# ========================================
# LUA CONFIGURATION (Architecture-aware)
# ========================================
option(ENABLE_SCRIPTING "Enable Lua/Sol3 scripting support" ON)

if(ENABLE_SCRIPTING)
    # Sol3 header-only library
    set(SOL3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sol3" CACHE PATH "Path to Sol3 include directory")
    if(NOT EXISTS "${SOL3_INCLUDE_DIR}/sol/sol.hpp")
        message(WARNING "Sol3 not found at ${SOL3_INCLUDE_DIR}. Scripting features will be disabled.")
        set(ENABLE_SCRIPTING OFF)
    endif()
endif()

if(ENABLE_SCRIPTING)
    # Detect architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
        set(IS_ARM64 TRUE)
        message(STATUS "🔧 Detected ARM64 architecture")
    else()
        set(IS_ARM64 FALSE)
        message(STATUS "🔧 Detected x86_64/other architecture")
    endif()

    # Platform-specific Lua detection
    if(APPLE)
        # macOS: Use Homebrew Lua with pkg-config for reliable detection
        find_package(PkgConfig QUIET)
        
        if(IS_ARM64)
            # Apple Silicon: Homebrew installs to /opt/homebrew
            set(HOMEBREW_PREFIX "/opt/homebrew")
        else()
            # Intel Mac: Homebrew installs to /usr/local
            set(HOMEBREW_PREFIX "/usr/local")
        endif()
        
        # Try pkg-config first (most reliable on macOS)
        if(PKG_CONFIG_FOUND)
            # Set PKG_CONFIG_PATH for Homebrew
            set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
            pkg_check_modules(LUA QUIET lua5.4 lua-5.4 lua54 lua5.3 lua-5.3 lua53 lua)
        endif()
        
        if(NOT LUA_FOUND)
            # Fallback: Try to find Lua manually in Homebrew paths
            find_path(LUA_INCLUDE_DIR lua.h
                PATHS
                    ${HOMEBREW_PREFIX}/include
                    ${HOMEBREW_PREFIX}/include/lua5.4
                    ${HOMEBREW_PREFIX}/include/lua
                    ${HOMEBREW_PREFIX}/opt/lua/include
                    ${HOMEBREW_PREFIX}/opt/lua@5.4/include
                NO_DEFAULT_PATH
            )
            
            find_library(LUA_LIBRARY
                NAMES lua lua5.4 lua54 lua-5.4
                PATHS
                    ${HOMEBREW_PREFIX}/lib
                    ${HOMEBREW_PREFIX}/opt/lua/lib
                    ${HOMEBREW_PREFIX}/opt/lua@5.4/lib
                NO_DEFAULT_PATH
            )
            
            if(LUA_INCLUDE_DIR AND LUA_LIBRARY)
                set(LUA_FOUND TRUE)
                set(LUA_LIBRARIES ${LUA_LIBRARY})
                message(STATUS "✅ Found Lua via Homebrew (manual): ${LUA_LIBRARY}")
                message(STATUS "   Include: ${LUA_INCLUDE_DIR}")
            endif()
        else()
            # pkg-config found it
            set(LUA_INCLUDE_DIR ${LUA_INCLUDE_DIRS})
            message(STATUS "✅ Found Lua via pkg-config: ${LUA_LIBRARIES}")
            message(STATUS "   Include: ${LUA_INCLUDE_DIR}")
        endif()
        
    elseif(UNIX)
        # Linux: Try pkg-config first, then find_package
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(LUA QUIET lua5.4 lua-5.4 lua54 lua5.3 lua-5.3 lua53 lua)
        endif()
        
        if(NOT LUA_FOUND)
            find_package(Lua QUIET)
        endif()
        
        if(LUA_FOUND)
            message(STATUS "✅ Found Lua on Linux: ${LUA_LIBRARIES}")
        endif()
        
    elseif(WIN32)
        # Windows: Try find_package, or look in common locations
        find_package(Lua QUIET)
        
        if(NOT LUA_FOUND)
            # Try common Windows Lua installation paths
            find_path(LUA_INCLUDE_DIR lua.h
                PATHS
                    "C:/Program Files/Lua/include"
                    "C:/Lua/include"
                    "$ENV{LUA_DIR}/include"
            )
            
            find_library(LUA_LIBRARY
                NAMES lua54 lua53 lua
                PATHS
                    "C:/Program Files/Lua/lib"
                    "C:/Lua/lib"
                    "$ENV{LUA_DIR}/lib"
            )
            
            if(LUA_INCLUDE_DIR AND LUA_LIBRARY)
                set(LUA_FOUND TRUE)
                set(LUA_LIBRARIES ${LUA_LIBRARY})
            endif()
        endif()
        
        if(LUA_FOUND)
            message(STATUS "✅ Found Lua on Windows: ${LUA_LIBRARIES}")
        endif()
    else()
        # Other platforms: standard find_package
        find_package(Lua QUIET)
    endif()
    
    if(NOT LUA_FOUND)
        message(WARNING "⚠️ Lua not found. Scripting features will be disabled.")
        message(STATUS "   To install Lua:")
        if(APPLE)
            message(STATUS "     brew install lua")
        elseif(UNIX)
            message(STATUS "     sudo apt-get install liblua5.4-dev  # Debian/Ubuntu")
            message(STATUS "     sudo dnf install lua-devel          # Fedora")
        elseif(WIN32)
            message(STATUS "     Download from https://www.lua.org/download.html")
        endif()
        set(ENABLE_SCRIPTING OFF)
    endif()
else()
    message(STATUS "ℹ️ Scripting disabled by user option")
    set(LUA_FOUND FALSE)
endif()

# Final status
if(ENABLE_SCRIPTING AND LUA_FOUND)
    message(STATUS "🎯 Scripting ENABLED with Lua and Sol3")
else()
    set(LUA_FOUND FALSE)
    message(STATUS "🎯 Scripting DISABLED")
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
        src/scripting/PrefabManager.cpp
        src/scripting/ScriptSystem.cpp
        src/scripting/UIBindings.cpp
    )

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

target_include_directories(MovementSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MovementSystem PUBLIC ecs)

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

# NOTE: Weapon, Enemy Spawn, and Movement Pattern systems moved to shootemup module
# See engine/modules/shootemup/

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

# UI System - for menus and UI elements (only if Lua/scripting is available)
if(LUA_FOUND)
    add_library(UISystem SHARED
        src/systems/UISystem.cpp
        src/systems/UISystemRender.cpp
        src/systems/UISystemEntities.cpp
    )

    target_include_directories(UISystem PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LUA_INCLUDE_DIR}
        ${SOL3_INCLUDE_DIR}
    )

    # Link Lua library - use the detected library from architecture-aware detection
    target_link_libraries(UISystem PUBLIC ecs rendering scripting ${LUA_LIBRARIES})
else()
    message(STATUS "UISystem disabled - requires Lua/Sol3")
endif()

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

# ========================================
# OPTIONAL MODULES
# ========================================

# Shoot'em Up Module (optional, can be disabled)
option(BUILD_SHOOTEMUP_MODULE "Build the Shoot'em Up game module" ON)

if(BUILD_SHOOTEMUP_MODULE)
    message(STATUS "🎮 Building Shoot'em Up module")
    add_subdirectory(modules/shootemup)
endif()
