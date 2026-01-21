# ==================== Game Logic Library ====================
# Logique métier spécifique au R-Type

# Scripting is always enabled - Lua/Sol3 required
set(SOL3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../engine/external/sol3" CACHE PATH "Path to Sol3")

# Use LUA_FOUND from parent if available, otherwise find Lua
if(NOT DEFINED LUA_FOUND OR NOT LUA_FOUND)
    find_package(Lua REQUIRED)
endif()

# Collect all source files
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/states/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gameplay/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp"
)

# Exclude old files to avoid conflicts
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Game\\.cpp$")  # Exclude old monolithic Game.cpp

if(GAME_SOURCES)
    add_library(game STATIC ${GAME_SOURCES})

    target_include_directories(game PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/engine/modules  # For shootemup module includes
        ${CMAKE_SOURCE_DIR}/server/include  # For shared network protocol (RTypeProtocol.hpp)
        ${SOL3_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
    )

    # Game dépend de l'engine + shootemup module
    target_link_libraries(game PUBLIC
        engine
        scripting
        shootemup
        ${LUA_LIBRARIES}
    )

    # Enable debug symbols for development
    target_compile_options(game PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:RelWithDebInfo>:-g>
    )

    # Define preprocessor flags
    target_compile_definitions(game PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )

    message(STATUS "Game library created with ${list_length_result} source files")
else()
    # Placeholder si pas de fichiers game encore
    add_library(game INTERFACE)

    target_include_directories(game INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/engine/modules  # For shootemup module includes
    )

    target_link_libraries(game INTERFACE
        engine
        shootemup
    )
    
    message(WARNING "No game source files found, creating interface library")
endif()

# Helper to get source count
list(LENGTH GAME_SOURCES list_length_result)

# ==================== Game Executable ====================
add_executable(r-type_game main.cpp)

target_link_libraries(r-type_game PRIVATE
    # Core engine libraries
    engine
    game
    core
    network
    scripting
    
    # Generic engine systems
    MovementSystem
    AnimationSystem
    StateMachineAnimationSystem
    LifetimeSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    UISystem
    AudioSystem
    
    # Shoot'em up module (contains WeaponSystem, MovementPatternSystem, etc.)
    shootemup
    
    # SFML
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)
