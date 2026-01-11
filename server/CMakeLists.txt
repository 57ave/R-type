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

# Link contre engine et game (pas besoin de SFML graphics pour le serveur)
target_link_libraries(r-type_server PRIVATE
    engine
    game
)

# Le serveur n'a pas besoin du rendering graphique
target_compile_definitions(r-type_server PRIVATE NO_GRAPHICS)
