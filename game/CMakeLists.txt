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
        ${SOL3_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
    )

    # Game dépend de l'engine
    target_link_libraries(game PUBLIC
        engine
        scripting
        ${LUA_LIBRARIES}
    )
else()
    # Placeholder si pas de fichiers game encore
    add_library(game INTERFACE)

    target_include_directories(game INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(game INTERFACE
        engine
    )
endif()

# ==================== Game Executable ====================
add_executable(r-type_game main.cpp)

target_link_libraries(r-type_game PRIVATE
    engine
    game
    core
    network
    scripting
    MovementSystem
    CollisionSystem
    AnimationSystem
    StateMachineAnimationSystem
    LifetimeSystem
    MovementPatternSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)
