# ==================== Server Executable ====================

# Choose which server to build (use main_improved.cpp for full game server)
option(USE_IMPROVED_SERVER "Use improved game server" ON)

if(USE_IMPROVED_SERVER)
    add_executable(r-type_server
        src/main_improved.cpp
        src/ServerConfig.cpp
    )
else()
    add_executable(r-type_server
        src/main.cpp
    )
endif()

target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Lua/Sol2 for server config loading (link directly, NOT via scripting lib which needs SFML)
# LUA_INCLUDE_DIR and LUA_LIBRARY are set as CACHE variables by engine CMakeLists or find_package
if(LUA_INCLUDE_DIR AND LUA_LIBRARY)
    target_include_directories(r-type_server PRIVATE
        ${LUA_INCLUDE_DIR}
        ${SOL3_INCLUDE_DIR}
    )
    target_link_libraries(r-type_server PRIVATE ${LUA_LIBRARY} ${LUA_MATH_LIBRARY})
    target_compile_definitions(r-type_server PRIVATE SERVER_SCRIPTING_ENABLED=1)
else()
    # Fallback: try to find the 'lua' target (when Lua was fetched via CPM)
    if(TARGET lua)
        target_include_directories(r-type_server PRIVATE
            $<TARGET_PROPERTY:lua,INTERFACE_INCLUDE_DIRECTORIES>
            ${SOL3_INCLUDE_DIR}
        )
        target_link_libraries(r-type_server PRIVATE lua)
        target_compile_definitions(r-type_server PRIVATE SERVER_SCRIPTING_ENABLED=1)
    else()
        target_compile_definitions(r-type_server PRIVATE SERVER_SCRIPTING_ENABLED=0)
    endif()
endif()

# Link contre engine (game est un exécutable, pas une lib)
target_link_libraries(r-type_server PRIVATE
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

# Copy Lua config files to server build directory
add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:r-type_server>/assets/scripts/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/game/assets/scripts/config
        $<TARGET_FILE_DIR:r-type_server>/assets/scripts/config
    COMMENT "Copying Lua config files to server build directory"
)

if(WIN32)
    add_custom_command(TARGET r-type_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-network>
            $<TARGET_FILE_DIR:r-type_server>
    )

    # Copy dependencies
    add_custom_command(TARGET r-type_server POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=$<TARGET_FILE_DIR:sfml-network>
            -DDEST_DIR=$<TARGET_FILE_DIR:r-type_server>
            -P ${CMAKE_SOURCE_DIR}/scripts/CopyRuntimeDlls.cmake
    )

    # Copy lua.dll (built as shared library on Windows)
    if(TARGET lua)
        add_custom_command(TARGET r-type_server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:lua>
                $<TARGET_FILE_DIR:r-type_server>
            COMMENT "Copying lua.dll to server output directory"
        )
    endif()

    # Copy engine system DLLs (all built as SHARED on Windows)
    foreach(_sys
        MovementSystem
        AnimationSystem
        StateMachineAnimationSystem
        UISystem
        AudioSystem
        LifetimeSystem
        BoundarySystem
        HealthSystem
        ScrollingBackgroundSystem
    )
        if(TARGET ${_sys})
            add_custom_command(TARGET r-type_server POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${_sys}>
                    $<TARGET_FILE_DIR:r-type_server>
                COMMENT "Copying ${_sys}.dll to server output directory"
            )
        endif()
    endforeach()
endif()
