# ==================== Server Executable ====================

# Choose which server to build (use main_improved.cpp for full game server)
option(USE_IMPROVED_SERVER "Use improved game server" ON)

if(USE_IMPROVED_SERVER)
    add_executable(r-type_server
        src/main_improved.cpp
    )
else()
    add_executable(r-type_server
        src/main.cpp
    )
endif()

target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link contre game et engine
target_link_libraries(r-type_server PRIVATE
    game
    engine
    MovementSystem
    AnimationSystem
    StateMachineAnimationSystem
    LifetimeSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    shootemup
)

# UISystem et AudioSystem sont optionnels (dépendent de Lua)
if(TARGET UISystem)
    target_link_libraries(r-type_server PRIVATE UISystem)
endif()
if(TARGET AudioSystem)
    target_link_libraries(r-type_server PRIVATE AudioSystem)
endif()

# Le serveur n'a pas besoin du rendering graphique
target_compile_definitions(r-type_server PRIVATE NO_GRAPHICS)

install(TARGETS r-type_server
    RUNTIME DESTINATION bin
)
