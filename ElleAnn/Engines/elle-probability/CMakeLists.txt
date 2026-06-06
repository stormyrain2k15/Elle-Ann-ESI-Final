cmake_minimum_required(VERSION 3.20)
project(elle_probability VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# C++17 required. Strict warnings.
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()

# ============================================================================
# Dependencies (fetched -- no manual third-party setup)
# ============================================================================
include(FetchContent)

# nlohmann/json -- same as language engine
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

# doctest -- same as language engine
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json doctest)

# ============================================================================
# elle_probability_core -- the engine library
# ============================================================================
add_library(elle_probability_core STATIC
    src/Types.cpp
    src/BayesianUpdater.cpp
    src/BeliefStore.cpp
    src/SenseProbabilityResolver.cpp
    src/IntentAnalyzer.cpp
    src/EmotionalPosteriorBuilder.cpp
    src/SpeakerTrustModel.cpp
    src/ProbabilityEngine.cpp
    src/AuditTrace.cpp
)

target_include_directories(elle_probability_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(elle_probability_core
    PUBLIC
        nlohmann_json::nlohmann_json
)

# Thread support (BeliefStore owns threads).
find_package(Threads REQUIRED)
target_link_libraries(elle_probability_core PUBLIC Threads::Threads)

# ============================================================================
# prob_heartbeat_demo
# ============================================================================
add_executable(prob_heartbeat_demo
    apps/prob_heartbeat_demo.cpp
)
target_link_libraries(prob_heartbeat_demo PRIVATE elle_probability_core)

# ============================================================================
# Tests
# ============================================================================
enable_testing()

add_executable(elle_prob_tests
    tests/test_main.cpp
    tests/test_distribution.cpp
    tests/test_bayesian_updater.cpp
    tests/test_belief_store.cpp
    tests/test_sense_resolver.cpp
    tests/test_intent_analyzer.cpp
    tests/test_emotional_posterior.cpp
    tests/test_speaker_trust.cpp
    tests/test_engine_integration.cpp
)

target_link_libraries(elle_prob_tests
    PRIVATE
        elle_probability_core
        doctest::doctest
)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(elle_prob_tests)

# ============================================================================
# Install
# ============================================================================
install(TARGETS elle_probability_core
    EXPORT elle_probability_targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
