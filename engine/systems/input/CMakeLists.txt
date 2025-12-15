cmake_minimum_required(VERSION 3.15)
project(InputSystem VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/InputSystem.cpp
)

add_library(InputSystem SHARED ${SOURCES})

target_include_directories(InputSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(InputSystem PUBLIC ecs)

set_target_properties(InputSystem PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PREFIX "lib"
    SUFFIX ".so"
)

install(TARGETS InputSystem
    LIBRARY DESTINATION lib
)
