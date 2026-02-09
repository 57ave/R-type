cmake_minimum_required(VERSION 3.14)

project(RType VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Audio Network REQUIRED)
find_package(Boost REQUIRED)

add_executable(r-type_client src/client/main.cpp)
target_link_libraries(r-type_client PRIVATE SFML::Graphics SFML::Audio SFML::Network Boost::boost)

add_executable(r-type_server src/server/main.cpp)
target_link_libraries(r-type_server PRIVATE SFML::Network Boost::boost)

if(WIN32)
    add_custom_command(TARGET r-type_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::Graphics>
            $<TARGET_FILE:SFML::Window>
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE:SFML::Audio>
            $<TARGET_FILE:SFML::Network>
            $<TARGET_FILE_DIR:r-type_client>
    )
    add_custom_command(TARGET r-type_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SFML::System>
            $<TARGET_FILE:SFML::Network>
            $<TARGET_FILE_DIR:r-type_server>
    )
endif()
