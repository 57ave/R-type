cmake_minimum_required(VERSION 3.14)

project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Toolchain is handled by CMakePresets.json or build scripts
# if(WIN32)
#     set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
# endif()

find_package(SFML 3 COMPONENTS Graphics Audio Network REQUIRED)
find_package(Boost REQUIRED)

add_executable(RType src/main.cpp)

target_link_libraries(RType PRIVATE SFML::Graphics SFML::Audio SFML::Network Boost::boost)

if(WIN32)
    add_custom_command(TARGET RType POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::Graphics>
            $<TARGET_FILE:SFML::Window>
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE:SFML::Audio>
            $<TARGET_FILE:SFML::Network>
            $<TARGET_FILE_DIR:RType>
    )
endif()
