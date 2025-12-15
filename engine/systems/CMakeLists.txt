cmake_minimum_required(VERSION 3.15)

# Build all system libraries
add_subdirectory(movement)
add_subdirectory(collision)
add_subdirectory(lifetime)
add_subdirectory(input)

message(STATUS "Building R-Type System Libraries:")
message(STATUS "  - MovementSystem")
message(STATUS "  - AISystem")
message(STATUS "  - LifetimeSystem")
message(STATUS "  - RenderSystem")
