cmake_minimum_required(VERSION 3.15)
project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options de build
option(BUILD_CLIENT "Build client executable" ON)
option(BUILD_SERVER "Build server executable" ON)
option(BUILD_TESTS "Build tests" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE SOURCES engine/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# Ajoute le dossier include au chemin de recherche
target_include_directories(${PROJECT_NAME} PRIVATE engine/include)

# Trouve et lie SFML
find_package(SFML 2.5 COMPONENTS system window graphics audio network REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
