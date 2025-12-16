# ==================== Client Executable ====================

# Sources du client (implémentations client-specific)
file(GLOB_RECURSE CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Créer l'exécutable client
add_executable(r-type_client
    ${CLIENT_SOURCES}
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link contre engine et game
target_link_libraries(r-type_client PRIVATE
    engine
    game
)
