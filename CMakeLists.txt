cmake_minimum_required(VERSION 3.16)
project(rtype_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SFML
  GITHUB_REPOSITORY SFML/SFML
  GIT_TAG 2.6.1
  GIT_SHALLOW TRUE
)

CPMAddPackage("gh:chriskohlhoff/asio#asio-1-28-0")

add_executable(r-type_server
    engine/src/network/NetworkServer.cpp
)

find_package(SFML 2.6.1 REQUIRED)

target_include_directories(r-type_server PRIVATE ${asio_SOURCE_DIR}/include engine/src)
target_link_libraries(r-type_server PRIVATE sfml-graphics sfml-window sfml-system)

