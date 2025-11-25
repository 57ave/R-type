cmake_minimum_required(VERSION 3.14)
project(ECS VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
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
    src/EntityManager.cpp
    src/ComponentManager.cpp
    src/SystemManager.cpp
    src/Coordinator.cpp
)

target_include_directories(ecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Example executable (optional)
option(BUILD_ECS_EXAMPLES "Build ECS examples" ON)

if(BUILD_ECS_EXAMPLES)
    add_executable(ecs_example
        examples/main.cpp
    )
    
    add_executable(test_network_id
        examples/test_network_id.cpp
    )
    
    target_link_libraries(ecs_example PRIVATE ecs)
    target_link_libraries(test_network_id PRIVATE ecs)
endif()

# Testing (optional)
option(BUILD_ECS_TESTS "Build ECS tests" OFF)

if(BUILD_ECS_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
