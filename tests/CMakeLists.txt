include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage("gh:google/googletest#v1.14.0")

enable_testing()

add_executable(unit_tests
    NetworkTests.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/include
)

target_link_libraries(unit_tests PRIVATE
    engine
    gtest_main
)

add_test(NAME AllTests COMMAND unit_tests)
