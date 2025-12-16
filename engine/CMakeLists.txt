# ==================== Engine Library ====================
# Collecte tous les fichiers sources de l'engine
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ecs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp"
)

# Inclure CPM.cmake pour la gestion des dépendances
include(cmake/CPM.cmake)

# Dépendance ASIO
CPMAddPackage("gh:chriskohlhoff/asio#asio-1-28-0")
# Dépendance SFML: prefer system package, fallback to CPM
find_package(SFML 2.6 COMPONENTS graphics window system QUIET)
if (NOT SFML_FOUND)
    CPMAddPackage("gh:SFML/SFML#2.6.0")
endif()

# Crée une seule library statique avec tout l'engine
add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${asio_SOURCE_DIR}/asio/include
)

target_link_libraries(engine PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
)

# ==================== Examples (Optional) ====================
option(BUILD_ENGINE_EXAMPLES "Build engine examples" OFF)

if(BUILD_ENGINE_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==================== Installation ====================
install(TARGETS engine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
