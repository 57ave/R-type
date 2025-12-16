# ==================== Server Executable ====================

# Sources du serveur
file(GLOB_RECURSE SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Créer l'exécutable serveur
add_executable(r-type_server
    ${SERVER_SOURCES}
)

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
