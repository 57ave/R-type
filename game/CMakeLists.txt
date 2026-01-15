# ==================== Game Logic Library ====================
# Logique métier spécifique au R-Type

# Trouver Lua et Sol3
set(SOL3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../engine/external/sol3/include" CACHE PATH "Path to Sol3")
find_package(Lua REQUIRED)

file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
)

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
endif()

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
    
    # Shoot'em up module (contains WeaponSystem, MovementPatternSystem, etc.)
    shootemup
    
    # SFML
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)
