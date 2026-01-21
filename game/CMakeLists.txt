# ==================== Game Logic Library ====================
# Logique métier spécifique au R-Type

# Scripting dependencies are inherited from engine
# Ensure we have access to variables if needed
if(NOT SOL3_INCLUDE_DIR AND TARGET sol2)
    get_target_property(SOL3_INCLUDE_DIR sol2 INTERFACE_INCLUDE_DIRECTORIES)
endif()

file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp"
)

if(GAME_SOURCES)
    add_library(game STATIC ${GAME_SOURCES})

    target_include_directories(game PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/engine/modules  # For shootemup module includes
        ${CMAKE_SOURCE_DIR}/server/include  # For shared network protocol (RTypeProtocol.hpp)
    )

    if(SOL3_INCLUDE_DIR)
        target_include_directories(game PUBLIC ${SOL3_INCLUDE_DIR})
    endif()
    if(LUA_INCLUDE_DIR)
        target_include_directories(game PUBLIC ${LUA_INCLUDE_DIR})
    endif()

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
    core
    network
    scripting
    game
    
    # Generic engine systems (UISystem must come after game)
    UISystem
    MovementSystem
    AnimationSystem
    StateMachineAnimationSystem
    LifetimeSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    AudioSystem
    
    # Shoot'em up module (contains WeaponSystem, MovementPatternSystem, etc.)
    shootemup
    
    # SFML
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# Install steps
install(TARGETS r-type_game
    RUNTIME DESTINATION bin
)

# Install assets
# 1. Copy root assets (scripts)
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION bin
    )
endif()

# 2. Copy game-specific assets (fonts, images, sounds)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    # Note: Using "/" at the end of the source directory copies the *contents*
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/"
        DESTINATION bin/assets
    )
endif()

# 3. Copy client assets if they exist and are different
if(EXISTS "${CMAKE_SOURCE_DIR}/client/assets")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/client/assets/"
        DESTINATION bin/assets
    )
endif()

