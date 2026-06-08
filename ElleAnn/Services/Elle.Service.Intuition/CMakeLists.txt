cmake_minimum_required(VERSION 3.20)
project(elle_intuition VERSION 0.1.0 LANGUAGES CXX)

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

add_library(elle_intuition_core INTERFACE)

target_include_directories(elle_intuition_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

find_package(Threads REQUIRED)
target_link_libraries(elle_intuition_core INTERFACE Threads::Threads)

enable_testing()

add_executable(elle_intu_tests
    tests/test_main.cpp
    tests/test_patterns.cpp
    tests/test_fire_instincts.cpp
    tests/test_synthesize.cpp
    tests/test_combined_signal.cpp
    tests/test_feedback.cpp
)

target_link_libraries(elle_intu_tests
    PRIVATE
        elle_intuition_core
        doctest::doctest
)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(elle_intu_tests)
