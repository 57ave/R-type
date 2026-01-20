cmake_minimum_required(VERSION 3.15)

# On macOS, SFML requires Objective-C/C++ support
if(APPLE)
    project(RType VERSION 1.0.0 LANGUAGES CXX C OBJC OBJCXX)
else()
    project(RType VERSION 1.0.0 LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options de build
option(BUILD_CLIENT "Build client executable" ON)
option(BUILD_SERVER "Build server executable" ON)
option(BUILD_GAME "Build game executable" ON)
option(BUILD_TESTS "Build tests" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include CPM
include(cmake/CPM.cmake)

# Find SFML (required by all sub-projects)
find_package(SFML 2.6 COMPONENTS system window graphics audio network QUIET)
if(NOT SFML_FOUND)
    CPMAddPackage("gh:SFML/SFML#2.6.1")
endif()
add_subdirectory(engine)

# In your main CMakeLists.txt, add this before building shootemup module
# Find or add sol2 (Lua binding library)
find_package(Lua 5.3 QUIET)
if(NOT Lua_FOUND)
    message(STATUS "Lua not found, installing via CPM")
    CPMAddPackage(
        NAME lua
        URL https://www.lua.org/ftp/lua-5.4.4.tar.gz
        VERSION 5.4.4
        OPTIONS "INSTALL_BINARIES OFF"
    )
endif()

# Add sol2
CPMAddPackage(
    NAME sol2
    GITHUB_REPOSITORY ThePhD/sol2
    VERSION 3.3.0
    GIT_TAG v3.3.0
)

if(sol2_ADDED)
    message(STATUS "sol2 added successfully")
endif()

# Then later when building shootemup, ensure it has access to sol2
if(BUILD_GAME)
    add_subdirectory(game)
endif()

# In engine/modules/shootemup/CMakeLists.txt, add:
target_include_directories(shootemup PRIVATE
    ${sol2_SOURCE_DIR}/include
    ${LUA_INCLUDE_DIR}
)

target_link_libraries(shootemup PRIVATE
    ${LUA_LIBRARIES}
)

# Fix for sol2/Lua dependency issue
if(BUILD_GAME)
    # Try to find system Lua first
    find_package(Lua 5.3 QUIET)
    
    if(NOT Lua_FOUND)
        message(STATUS "System Lua not found, downloading via CPM")
        CPMAddPackage(
            NAME lua
            URL https://www.lua.org/ftp/lua-5.4.4.tar.gz
            VERSION 5.4.4
            OPTIONS
                "INSTALL_BINARIES OFF"
                "BUILD_SHARED_LIBS OFF"
        )
        if(lua_ADDED)
            set(LUA_INCLUDE_DIR ${lua_SOURCE_DIR}/src)
            set(LUA_LIBRARIES lua)
        endif()
    endif()
    
    # Add sol2
    CPMAddPackage(
        NAME sol2
        GITHUB_REPOSITORY ThePhD/sol2
        VERSION 3.3.0
        OPTIONS
            "SOL2_ENABLE_INSTALL OFF"
            "SOL2_BUILD_EXAMPLES OFF"
    )
    
    if(sol2_ADDED AND (LUA_INCLUDE_DIR OR lua_ADDED))
        message(STATUS "sol2 and Lua dependencies resolved")
        
        # Add these includes to the engine module that needs them
        if(TARGET shootemup)
            target_include_directories(shootemup PRIVATE
                ${sol2_SOURCE_DIR}/include
                ${LUA_INCLUDE_DIR}
            )
            
            if(LUA_LIBRARIES)
                target_link_libraries(shootemup PRIVATE ${LUA_LIBRARIES})
            elseif(lua_ADDED)
                target_link_libraries(shootemup PRIVATE lua)
            endif()
        endif()
    else()
        message(WARNING "sol2 or Lua dependencies not found. Game module may not build correctly.")
    endif()
endif()

# Build client if requested
if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

# Build server if requested
if(BUILD_SERVER)
    add_subdirectory(server)
endif()

# Build game if requested
if(BUILD_GAME)
    add_subdirectory(game)
endif()
