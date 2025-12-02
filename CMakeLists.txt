cmake_minimum_required(VERSION 3.16)
project(rtype_server)

set(CMAKE_CXX_STANDARD 17)
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME SFML
  GITHUB_REPOSITORY SFML/SFML
  GIT_TAG 2.6.1
  GIT_SHALLOW TRUE
)

## Use CPM to fetch Boost instead of FetchContent
CPMAddPackage(
  NAME Boost
  GITHUB_REPOSITORY boostorg/boost
  GIT_TAG boost-1.84.0
  GIT_SHALLOW TRUE
)

## add boost cmake module
## (Optional) boost cmake helpers removed — use system Boost via FindBoost

add_executable(r-type_server
    engine/src/network/NetworkServer.cpp
)

find_package(SFML 2.6.1 REQUIRED)

find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem)
target_link_libraries(r-type_server PRIVATE Boost::system Boost::asio Boost::filesystem sfml-graphics sfml-window sfml-system)

