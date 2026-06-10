cmake_minimum_required(VERSION 3.20)
project(elle_shared_tests VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(doctest)

enable_testing()

add_executable(elle_shared_tests
    tests/test_main.cpp
    tests/test_upload_guard.cpp
    tests/test_intent_label_vocab.cpp
    tests/test_sql_fallback_classifier.cpp
    ../ElleUploadGuard.cpp
)
target_include_directories(elle_shared_tests PRIVATE ..)
target_link_libraries(elle_shared_tests PRIVATE doctest::doctest)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(elle_shared_tests)
