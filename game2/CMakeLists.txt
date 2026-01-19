cmake_minimum_required(VERSION 3.15)

project(FlappyBird)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files from src/
file(GLOB_RECURSE GAME2_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Collect all header files from include/
file(GLOB_RECURSE GAME2_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# Display found files for debugging
message(STATUS "Flappy Bird sources found: ${GAME2_SOURCES}")
message(STATUS "Flappy Bird headers found: ${GAME2_HEADERS}")

# Create executable from all source files in src/
add_executable(flappy_bird
    ${GAME2_SOURCES}
)

# Include directories
target_include_directories(flappy_bird PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/engine/modules/shootemup/include
)

# Link libraries (matching game/CMakeLists.txt structure)
target_link_libraries(flappy_bird PRIVATE
    # Core engine libraries
    engine
    core
    network
    scripting
    
    # Generic engine systems (individual libraries)
    MovementSystem
    AnimationSystem
    StateMachineAnimationSystem
    LifetimeSystem
    ScrollingBackgroundSystem
    BoundarySystem
    HealthSystem
    UISystem
    AudioSystem
    
    # Shoot'em up module (if needed)
    shootemup
    
    # SFML libraries
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

# Set output directory to project root for easy execution
set_target_properties(flappy_bird PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/game2
)

# Copy assets to build directory (if assets folder exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET flappy_bird POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_SOURCE_DIR}/game2/assets
        COMMENT "Copying game2 assets to output directory"
    )
endif()

# Enable warnings
if(MSVC)
    target_compile_options(flappy_bird PRIVATE /W4)
else()
    target_compile_options(flappy_bird PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "✅ Flappy Bird (game2) configured successfully")
