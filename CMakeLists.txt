cmake_minimum_required(VERSION 3.16)
project(MatchingEngine LANGUAGES CXX)

# --- 1) Options C++17 ------------------------------------------------------
set(CMAKE_CXX_STANDARD      17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS     OFF)

# (Pour macOS : activer le support rpath si besoin)
set(CMAKE_MACOSX_RPATH TRUE)

# --- 2) Core library --------------------------------------------------------
add_library(core STATIC
        src/CsvParser.cpp
        src/CsvWriter.cpp
        src/Order.cpp
        src/OrderBook.cpp
        src/MatchingEngine.cpp
        src/Logger.cpp
)
target_include_directories(core
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
target_compile_features(core
        PUBLIC
        cxx_std_17
)

# --- 3) Exécutable principal -----------------------------------------------
add_executable(app
        main.cpp
)
target_link_libraries(app
        PRIVATE core
)
target_compile_definitions(app
        PRIVATE DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
target_compile_features(app
        PRIVATE cxx_std_17
)

# --- 4) Bench executable ---------------------------------------------------
add_executable(Performance
        bench/Performance.cpp
)
target_link_libraries(Performance
        PRIVATE core
)
target_compile_features(Performance
        PRIVATE cxx_std_17
)

# --- 5) GoogleTest via FetchContent ----------------------------------------
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)
# Désactiver GMOCK
set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST   ON  CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- 6) Tests unitaires ----------------------------------------------------
enable_testing()

# Copie les CSV de tests dans build/tests/data avant de compiler les tests
add_custom_target(copy_test_data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tests/data
        ${CMAKE_BINARY_DIR}/tests/data
)

# Récupère tous les test_*.cpp
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/unit/test_*.cpp")
message(STATUS "TEST_SOURCES = ${TEST_SOURCES}")

# Pour chaque source de test, crée un exécutable et un test CTest
foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name}
            ${test_src}
    )
    target_link_libraries(${test_name}
            PRIVATE core GTest::gtest_main
    )
    target_include_directories(${test_name}
            PRIVATE ${PROJECT_SOURCE_DIR}/include
    )
    add_dependencies(${test_name} copy_test_data)

    add_test(
            NAME ${test_name}
            COMMAND ${test_name}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
