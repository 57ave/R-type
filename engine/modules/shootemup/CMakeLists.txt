# Shoot'em Up Module for R-Type Engine
# Reusable components, systems, and utilities for shoot'em up games

cmake_minimum_required(VERSION 3.20)

project(shootemup_module VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
# COLLECT SOURCE FILES
# ========================================

# Components (header-only)
set(SHOOTEMUP_COMPONENT_HEADERS
    include/components/Weapon.hpp
    include/components/MovementPattern.hpp
    include/components/Attachment.hpp
    include/components/ShootEmUpTags.hpp
    include/components/PowerUp.hpp
    include/components/AIController.hpp
    include/components/Boss.hpp
    include/components/ForcePod.hpp
    include/components/Wave.hpp
    include/components/Effect.hpp
)

# Systems
set(SHOOTEMUP_SYSTEM_HEADERS
    include/systems/WeaponSystem.hpp
    include/systems/MovementPatternSystem.hpp
    include/systems/EnemySpawnSystem.hpp
    include/systems/WaveSystem.hpp
    include/systems/BossSystem.hpp
    include/systems/AttachmentSystem.hpp
)

set(SHOOTEMUP_SYSTEM_SOURCES
    src/systems/WeaponSystem.cpp
    src/systems/MovementPatternSystem.cpp
    src/systems/EnemySpawnSystem.cpp
    src/systems/WaveSystem.cpp
    src/systems/BossSystem.cpp
    src/systems/AttachmentSystem.cpp
)

# Factories
set(SHOOTEMUP_FACTORY_HEADERS
    include/factories/EnemyFactory.hpp
    include/factories/ProjectileFactory.hpp
)

set(SHOOTEMUP_FACTORY_SOURCES
    src/factories/EnemyFactory.cpp
    src/factories/ProjectileFactory.cpp
)

# ========================================
# CREATE LIBRARY
# ========================================

add_library(shootemup STATIC
    ${SHOOTEMUP_SYSTEM_SOURCES}
    ${SHOOTEMUP_FACTORY_SOURCES}
)

# Alias for consistent naming
add_library(engine::shootemup ALIAS shootemup)

# ========================================
# INCLUDE DIRECTORIES
# ========================================

# Make shootemup headers available as <components/...>, <systems/...>, <factories/...>
target_include_directories(shootemup PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link to engine core components
target_include_directories(shootemup PUBLIC
    ${CMAKE_SOURCE_DIR}/engine/include
    ${LUA_INCLUDE_DIR}
    ${SOL3_INCLUDE_DIR}
)

# ========================================
# DEPENDENCIES
# ========================================

# Link to engine core libraries
target_link_libraries(shootemup PUBLIC
    ecs
    rendering
)

# SFML for rendering (if needed)
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
if(SFML_FOUND)
    target_link_libraries(shootemup PRIVATE sfml-graphics sfml-window sfml-system)
endif()

# ========================================
# COMPILER OPTIONS
# ========================================

if(MSVC)
    target_compile_options(shootemup PRIVATE /W4)
else()
    target_compile_options(shootemup PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ========================================
# INSTALLATION (optional)
# ========================================

install(TARGETS shootemup
    EXPORT shootemup-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/shootemup
)

message(STATUS "✅ Shoot'em Up module configured successfully")
message(STATUS "   Components: ${CMAKE_CURRENT_SOURCE_DIR}/include/components")
message(STATUS "   Systems: ${CMAKE_CURRENT_SOURCE_DIR}/include/systems")
message(STATUS "   Factories: ${CMAKE_CURRENT_SOURCE_DIR}/include/factories")
