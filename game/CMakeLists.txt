# R-Type Game - CMakeLists.txt
cmake_minimum_required(VERSION 3.17)
project(rtype_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ignore Sol2 template warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-error=template-body)
endif()

message(STATUS "🎮 R-Type Game - Configuring...")

# Collect all source files
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Add main.cpp
set(GAME_SOURCES ${GAME_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
# Create game executable
add_executable(game ${GAME_SOURCES})

# Include directories
target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/engine/external/sol3/include
)

# Link against engine libraries
target_link_libraries(game PRIVATE
    # Core engine
    engine
    ecs
    core
    rendering
    network
    scripting
    
    # Systems (only those that exist as shared libraries)
    MovementSystem
    AnimationSystem
    StateMachineAnimationSystem
    UISystem
    AudioSystem
    InputSystem
    LifetimeSystem
    BoundarySystem
    HealthSystem
    ScrollingBackgroundSystem
    
    # SFML
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)

# Copy assets to build directory
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:game>/assets
    COMMENT "Copying game assets to build directory"
)

if(WIN32)
    # Copy SFML DLLs explicitly
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE:sfml-network>
            $<TARGET_FILE_DIR:game>
    )

    # Copy dependencies (freetype, flac, ogg, etc.)
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
            -DSOURCE_DIR=$<TARGET_FILE_DIR:sfml-graphics> 
            -DDEST_DIR=$<TARGET_FILE_DIR:game> 
            -P ${CMAKE_SOURCE_DIR}/scripts/CopyRuntimeDlls.cmake
    )
    
     # Copy OpenAL if needed
    if(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                $<TARGET_FILE_DIR:game>
        )
    elseif(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll")
         add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll"
                $<TARGET_FILE_DIR:game>
        )
    endif()

    # Copy lua.dll (built as shared library on Windows)
    if(TARGET lua)
        add_custom_command(TARGET game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:lua>
                $<TARGET_FILE_DIR:game>
            COMMENT "Copying lua.dll to game output directory"
        )
    endif()

    # Copy engine system DLLs (all built as SHARED on Windows)
    foreach(_sys
        MovementSystem
        AnimationSystem
        StateMachineAnimationSystem
        UISystem
        AudioSystem
        InputSystem
        LifetimeSystem
        BoundarySystem
        HealthSystem
        ScrollingBackgroundSystem
    )
        if(TARGET ${_sys})
            add_custom_command(TARGET game POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${_sys}>
                    $<TARGET_FILE_DIR:game>
                COMMENT "Copying ${_sys}.dll to game output directory"
            )
        endif()
    endforeach()
endif()

message(STATUS "✅ R-Type Game configured successfully")
message(STATUS "   Sources: ${CMAKE_CURRENT_SOURCE_DIR}/src")
message(STATUS "   Assets: ${CMAKE_CURRENT_SOURCE_DIR}/assets")
