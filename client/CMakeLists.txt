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

add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:r-type_client>/assets
    COMMENT "Copying client assets to build directory"
)

if(WIN32)
    # Copy SFML DLLs explicitly
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE:sfml-network>
            $<TARGET_FILE_DIR:r-type_client>
    )

    # Copy dependencies (freetype, flac, ogg, etc.)
    # Located in the same directory as sfml-graphics
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
            -DSOURCE_DIR=$<TARGET_FILE_DIR:sfml-graphics> 
            -DDEST_DIR=$<TARGET_FILE_DIR:r-type_client> 
            -P ${CMAKE_SOURCE_DIR}/scripts/CopyRuntimeDlls.cmake
    )

    # OpenAL might be in a different place (from OpenAL-Soft static build or SFML extlibs)
    # If using SFML's extlibs:
    if(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
        add_custom_command(TARGET r-type_client POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                $<TARGET_FILE_DIR:r-type_client>
        )
    elseif(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll")
         add_custom_command(TARGET r-type_client POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll"
                $<TARGET_FILE_DIR:r-type_client>
        )
    endif()
endif()
