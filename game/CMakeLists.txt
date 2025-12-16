# ==================== Game Logic Library ====================
# Logique métier spécifique au R-Type

file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/components/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
)

if(GAME_SOURCES)
    add_library(game STATIC ${GAME_SOURCES})
    
    target_include_directories(game PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    # Game dépend de l'engine
    target_link_libraries(game PUBLIC
        engine
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
