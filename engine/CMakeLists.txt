cmake_minimum_required(VERSION 3.15)
project(RTypeEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ECS Library
add_library(ecs STATIC
    src/ecs/EntityManager.cpp
    src/ecs/ComponentManager.cpp
    src/ecs/SystemManager.cpp
    src/ecs/Coordinator.cpp
)

target_include_directories(ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Network library (header-only for serialization)
add_library(network INTERFACE)

target_include_directories(network INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link network to ECS for Types.hpp
target_link_libraries(network INTERFACE ecs)

# Rendering library
add_library(rendering STATIC
    src/rendering/Window.cpp
    src/rendering/Camera.cpp
)

target_include_directories(rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: Build examples
option(BUILD_ENGINE_EXAMPLES "Build engine examples" OFF)

if(BUILD_ENGINE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(TARGETS ecs rendering network
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
