cmake_minimum_required(VERSION 3.20)
project(elle_engine
    VERSION 0.1.0
    DESCRIPTION "Elle's integer-indexed semantic language engine (non-LLM)"
    LANGUAGES CXX)

# -------------------------------------------------------------------------
# Compiler standards
# -------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
endif()

# -------------------------------------------------------------------------
# Options
# -------------------------------------------------------------------------
option(ELLE_BUILD_TESTS    "Build unit tests"                ON)
option(ELLE_BUILD_DEMOS    "Build heartbeat + smoke demos"   ON)
option(ELLE_WITH_ODBC      "Build the SQL Server ODBC layer" ON)

# -------------------------------------------------------------------------
# Third-party (header-only, fetched at configure time)
# -------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3)
FetchContent_MakeAvailable(json)

if(ELLE_BUILD_TESTS)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.11)
    FetchContent_MakeAvailable(doctest)
endif()

# -------------------------------------------------------------------------
# Core engine library
# -------------------------------------------------------------------------
set(ELLE_CORE_SOURCES
    src/StringUtil.cpp
    src/Config.cpp
    src/InputNormalizer.cpp
    src/PhraseScanner.cpp
    src/WordFormResolver.cpp
    src/IntegerSequenceBuilder.cpp
    src/SenseCandidateResolver.cpp
    src/ContextComparator.cpp
    src/EmotionalWeightProcessor.cpp
    src/SemanticGraphWalker.cpp
    src/MeaningObjectBuilder.cpp
    src/DebugTrace.cpp
    src/InMemoryAccessLayer.cpp
    src/Engine.cpp
)

add_library(elle_core STATIC ${ELLE_CORE_SOURCES})
target_include_directories(elle_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(elle_core PUBLIC nlohmann_json::nlohmann_json)

# -------------------------------------------------------------------------
# Optional ODBC / SQL Server layer
# -------------------------------------------------------------------------
if(ELLE_WITH_ODBC)
    if(WIN32)
        # SQL Server's preferred client lib on Windows. odbc32 ships in the SDK.
        set(ELLE_ODBC_LIBS odbc32)
    else()
        # unixODBC for Linux/macOS development builds. Production target is MSVC + odbc32.
        find_library(UNIXODBC_LIB NAMES odbc)
        if(UNIXODBC_LIB)
            set(ELLE_ODBC_LIBS ${UNIXODBC_LIB})
        else()
            message(WARNING "unixODBC not found; ODBC layer will be built header-only.")
            set(ELLE_ODBC_LIBS "")
        endif()
    endif()

    add_library(elle_odbc STATIC
        src/OdbcConnection.cpp
        src/SqlServerAccessLayer.cpp
    )
    target_include_directories(elle_odbc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(elle_odbc PUBLIC elle_core ${ELLE_ODBC_LIBS})
    target_compile_definitions(elle_odbc PUBLIC ELLE_HAVE_ODBC=1)
endif()

# -------------------------------------------------------------------------
# Demo executables
# -------------------------------------------------------------------------
if(ELLE_BUILD_DEMOS)
    add_executable(heartbeat_demo apps/heartbeat_demo.cpp)
    target_link_libraries(heartbeat_demo PRIVATE elle_core)

    add_executable(smoke_test apps/smoke_test.cpp)
    target_link_libraries(smoke_test PRIVATE elle_core)
endif()

# -------------------------------------------------------------------------
# Tests
# -------------------------------------------------------------------------
if(ELLE_BUILD_TESTS)
    enable_testing()
    add_executable(elle_tests
        tests/test_main.cpp
        tests/test_input_normalizer.cpp
        tests/test_homonym.cpp
        tests/test_homophone.cpp
        tests/test_phrase_resolution.cpp
        tests/test_emotional_context.cpp
        tests/test_synonym_antonym.cpp
        tests/test_paraphrase.cpp
        tests/test_unknown_word.cpp
        tests/test_phrase_first.cpp
        tests/test_context_scoring.cpp
        tests/test_semantic_graph.cpp
        tests/test_debug_trace.cpp
    )
    target_link_libraries(elle_tests PRIVATE elle_core doctest::doctest)
    add_test(NAME elle_tests COMMAND elle_tests)
endif()
