cmake_minimum_required(VERSION 3.15)
project(MovementSystem VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/MovementSystem.cpp
)

# Create shared library
add_library(MovementSystem SHARED ${SOURCES})

# Include directories
target_include_directories(MovementSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

# Link ECS library
target_link_libraries(MovementSystem PUBLIC ecs)

# Set output directory
set_target_properties(MovementSystem PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX "lib"
    SUFFIX ".so"
)

# Install
install(TARGETS MovementSystem
    LIBRARY DESTINATION lib
)
