cmake_minimum_required(VERSION 3.15)

project(FlappyBirdServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DISABLED: Using the main server in /server/src/flappy_server.cpp instead
# Sources
# set(SERVER_SOURCES
#     FlappyServer.cpp
# )

# NEW SIMPLE SERVER
# set(SIMPLE_SERVER_SOURCES
#     SimpleFlappyServer.cpp
# )

# Headers
# set(SERVER_HEADERS
#     FlappyProtocol.hpp
#     SimpleProtocol.hpp
# )

# Create executable
# add_executable(flappy_br_server ${SERVER_SOURCES} ${SERVER_HEADERS})

# # Include directories - include ASIO from the CPM cache if available
# target_include_directories(flappy_br_server PRIVATE
#     ${CMAKE_SOURCE_DIR}/engine/include
# )

# # If asio_SOURCE_DIR is set (from parent CPM), use it
# if(DEFINED asio_SOURCE_DIR)
#     target_include_directories(flappy_br_server PRIVATE
#         ${asio_SOURCE_DIR}/asio/include
#     )
# endif()

# # Link against engine network library (which already includes ASIO headers)
# target_link_libraries(flappy_br_server PRIVATE
#     network
# )

# ============================================================================
# DISABLED: Using the main server in /server/src/flappy_server.cpp instead
# ============================================================================
# add_executable(simple_flappy_server ${SIMPLE_SERVER_SOURCES} ${SERVER_HEADERS})

# # Include directories
# target_include_directories(simple_flappy_server PRIVATE
#     ${CMAKE_SOURCE_DIR}/engine/include
# )

# # If asio_SOURCE_DIR is set (from parent CPM), use it
# if(DEFINED asio_SOURCE_DIR)
#     target_include_directories(simple_flappy_server PRIVATE
#         ${asio_SOURCE_DIR}/asio/include
#     )
# endif()

# # Link libraries
# target_link_libraries(simple_flappy_server PRIVATE
#     network
#     core
# )

# # Set output directory
# set_target_properties(simple_flappy_server PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )

# message(STATUS "Simple Flappy Server configured")

# ============================================================================

# # Set output directory (old server)
# set_target_properties(flappy_br_server PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )

message(STATUS "FlappyBird Servers DISABLED - using /server/flappy_server instead")
