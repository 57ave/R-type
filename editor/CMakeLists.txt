cmake_minimum_required(VERSION 3.15)
project(RTypeEditor VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    )
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-error>)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CPM.cmake)

find_package(SFML 2.6 COMPONENTS system window graphics QUIET)
if(NOT SFML_FOUND)
    CPMAddPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 2.6.1
        OPTIONS
            "SFML_BUILD_AUDIO OFF"
            "SFML_BUILD_NETWORK OFF"
            "SFML_BUILD_EXAMPLES OFF"
            "SFML_BUILD_DOC OFF"
            "SFML_INSTALL_PKGCONFIG_FILES OFF"
    )
endif()

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.89.9
    DOWNLOAD_ONLY YES
)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
CPMAddPackage(
    NAME imgui-sfml
    GITHUB_REPOSITORY SFML/imgui-sfml
    GIT_TAG v2.6
    OPTIONS
        "IMGUI_SFML_FIND_SFML OFF"
        "IMGUI_SFML_IMGUI_DEMO ON"
)

find_package(Lua 5.4 QUIET)
if(NOT LUA_FOUND)
    CPMAddPackage(
        NAME Lua
        GITHUB_REPOSITORY lua/lua
        GIT_TAG v5.4.6
        DOWNLOAD_ONLY YES
    )

    if(Lua_ADDED OR EXISTS "${Lua_SOURCE_DIR}")
        file(GLOB LUA_SOURCES "${Lua_SOURCE_DIR}/*.c")
        list(REMOVE_ITEM LUA_SOURCES
            "${Lua_SOURCE_DIR}/lua.c"
            "${Lua_SOURCE_DIR}/luac.c"
            "${Lua_SOURCE_DIR}/onelua.c"
        )
        add_library(lua_editor STATIC ${LUA_SOURCES})
        target_include_directories(lua_editor PUBLIC ${Lua_SOURCE_DIR})
        set_target_properties(lua_editor PROPERTIES POSITION_INDEPENDENT_CODE ON)
        if(UNIX)
            target_compile_definitions(lua_editor PUBLIC LUA_USE_LINUX)
        endif()
        set(LUA_LIBRARIES lua_editor)
        set(LUA_INCLUDE_DIR ${Lua_SOURCE_DIR})
    endif()
endif()

CPMAddPackage(
    NAME sol2
    GITHUB_REPOSITORY ThePhD/sol2
    GIT_TAG v3.3.1
)

find_package(OpenGL REQUIRED)

add_executable(rtype_editor
    src/main.cpp
    src/Editor.cpp
    src/FolderBrowser.cpp
    src/LuaParser.cpp
    src/Serializer.cpp
    src/WavePanel.cpp
    src/SpawnTable.cpp
    src/Canvas.cpp
)

target_include_directories(rtype_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
)

target_link_libraries(rtype_editor PRIVATE
    ImGui-SFML::ImGui-SFML
    sfml-graphics
    sfml-window
    sfml-system
    ${LUA_LIBRARIES}
    sol2
    OpenGL::GL
)

target_compile_definitions(rtype_editor PRIVATE
    SOL_ALL_SAFETIES_ON=1
)

if(WIN32)
    # Copy SFML DLLs explicitly
    add_custom_command(TARGET rtype_editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE:sfml-network>
            $<TARGET_FILE_DIR:rtype_editor>
    )

    # Copy dependencies (freetype, flac, ogg, etc.)
    add_custom_command(TARGET rtype_editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
            -DSOURCE_DIR=$<TARGET_FILE_DIR:sfml-graphics> 
            -DDEST_DIR=$<TARGET_FILE_DIR:rtype_editor> 
            -P ${CMAKE_SOURCE_DIR}/scripts/CopyRuntimeDlls.cmake
    )
    
     # Copy OpenAL if needed
    if(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll")
        add_custom_command(TARGET rtype_editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x64/openal32.dll"
                $<TARGET_FILE_DIR:rtype_editor>
        )
    elseif(EXISTS "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll")
         add_custom_command(TARGET rtype_editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SFML_SOURCE_DIR}/extlibs/bin/x86/openal32.dll"
                $<TARGET_FILE_DIR:rtype_editor>
        )
    endif()

    # Copy ImGui-SFML DLL
    if(TARGET ImGui-SFML)
        add_custom_command(TARGET rtype_editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:ImGui-SFML>
                $<TARGET_FILE_DIR:rtype_editor>
            COMMENT "Copying ImGui-SFML DLL to rtype_editor output directory"
        )
    endif()

    # Copy lua.dll (built as shared library on Windows)
    if(TARGET lua)
        add_custom_command(TARGET rtype_editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:lua>
                $<TARGET_FILE_DIR:rtype_editor>
            COMMENT "Copying lua.dll to rtype_editor output directory"
        )
    endif()
endif()
