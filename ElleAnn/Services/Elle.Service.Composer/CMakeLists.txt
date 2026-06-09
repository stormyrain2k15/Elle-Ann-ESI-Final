cmake_minimum_required(VERSION 3.20)
project(elle_composer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(doctest)

add_library(elle_composer_core INTERFACE)

target_include_directories(elle_composer_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

find_package(Threads REQUIRED)
target_link_libraries(elle_composer_core INTERFACE Threads::Threads)

enable_testing()

add_executable(elle_composer_tests
    tests/test_main.cpp
    tests/test_parse_slots.cpp
    tests/test_score_frame.cpp
    tests/test_chain_integration.cpp
)

target_link_libraries(elle_composer_tests
    PRIVATE
        elle_composer_core
        doctest::doctest
)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(elle_composer_tests)
