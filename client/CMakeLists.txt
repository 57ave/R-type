# ==================== Client Executable ====================
# Client wrapper qui utilise GameRefactored

# Créer l'exécutable client
add_executable(r-type_client main.cpp)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_SOURCE_DIR}/game/include
)

# Link contre engine et game
target_link_libraries(r-type_client PRIVATE
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
