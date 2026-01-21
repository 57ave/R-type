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

# Build Options
option(BUILD_CLIENT "Build client executable" ON)
option(BUILD_SERVER "Build server executable" ON)
option(BUILD_GAME "Build game executable" ON)
option(BUILD_TESTS "Build tests" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Include CPM
include(cmake/CPM.cmake)

# Configure RPATH for installed executables
# This ensures executables find shared libraries in ../lib relative to them
# AND shared libraries find each other in standard locations
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib" "\$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


if(WIN32 OR MINGW)
    message(STATUS "Fetching OpenAL-Soft for Windows...")
    CPMAddPackage(
        NAME OpenAL
        GITHUB_REPOSITORY kcat/openal-soft
        GIT_TAG 1.23.1
        OPTIONS
            "ALSOFT_UTILS OFF"
            "ALSOFT_EXAMPLES OFF"
            "ALSOFT_TESTS OFF"
            "ALSOFT_INSTALL_EXAMPLES OFF"
            "LIBTYPE STATIC"
    )
    if(OpenAL_ADDED)
        set(OPENAL_LIBRARY OpenAL)
        set(OPENAL_INCLUDE_DIR ${OpenAL_SOURCE_DIR}/include)
        set(OPENAL_FOUND TRUE)
        set(OPENAL_LIBRARIES OpenAL)
    endif()
endif()

# Find SFML (required by all sub-projects)
find_package(SFML COMPONENTS Graphics Window System Audio Network CONFIG)
if(NOT SFML_FOUND)
    # Fix for 'optimized.lib' error: Force SFML to use its own internal dependencies
    # instead of picking up vcpkg's libraries which may have incompatible usage (keywords).
    set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)
    
    CPMAddPackage(
        NAME SFML
        GITHUB_REPOSITORY SFML/SFML
        GIT_TAG 2.6.1
        OPTIONS
            "SFML_INSTALL_PKGCONFIG_FILES OFF"
            "SFML_BUILD_EXAMPLES OFF"
            "SFML_BUILD_DOC OFF"
            "SFML_USE_SYSTEM_DEPS OFF" 
    )
endif()
add_subdirectory(engine)

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
