# ==================== Client Executable ====================
# ==================== Client Executable ====================

# Sources du client (implémentations client-specific)
file(GLOB_RECURSE CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Ensure the top-level main.cpp is included if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    list(APPEND CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
endif()

# Créer l'exécutable client
add_executable(r-type_client
    main.cpp
    ${CLIENT_SOURCES}
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link contre engine seulement (game est un exécutable séparé maintenant)
target_link_libraries(r-type_client PRIVATE
    engine
)

if(WIN32)
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE:sfml-network>
            $<TARGET_FILE_DIR:r-type_client>
    )
endif()
