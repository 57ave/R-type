cmake_minimum_required(VERSION 3.15)
project(CollisionSystem VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/CollisionSystem.cpp
)

add_library(CollisionSystem SHARED ${SOURCES})

target_include_directories(CollisionSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(CollisionSystem PUBLIC ecs)

set_target_properties(CollisionSystem PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX "lib"
    SUFFIX ".so"
)

install(TARGETS CollisionSystem
    LIBRARY DESTINATION lib
)
