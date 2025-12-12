cmake_minimum_required(VERSION 3.16)

project(r-type-temporary VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4 /permissive-)
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
