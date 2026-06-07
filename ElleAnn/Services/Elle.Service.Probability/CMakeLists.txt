cmake_minimum_required(VERSION 3.20)
project(elle_probability VERSION 0.1.0 LANGUAGES CXX)

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
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json doctest)

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

find_package(Threads REQUIRED)
target_link_libraries(elle_probability_core PUBLIC Threads::Threads)

add_executable(prob_heartbeat_demo
    apps/prob_heartbeat_demo.cpp
)
target_link_libraries(prob_heartbeat_demo PRIVATE elle_probability_core)

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

set(ELLE_PROB_WITH_LANGUAGE_ENGINE "AUTO" CACHE STRING
    "Build the language-engine bridge (ON / OFF / AUTO)")
set(ELLE_LANGUAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Elle.Service.Language" CACHE PATH
    "Path to the Elle.Service.Language source tree (must contain include/elle/Types.hpp)")

set(_elle_lang_headers_found FALSE)
if(EXISTS "${ELLE_LANGUAGE_DIR}/include/elle/Types.hpp"
   AND EXISTS "${ELLE_LANGUAGE_DIR}/include/elle/Config.hpp")
    set(_elle_lang_headers_found TRUE)
endif()

if(ELLE_PROB_WITH_LANGUAGE_ENGINE STREQUAL "ON" OR
   (ELLE_PROB_WITH_LANGUAGE_ENGINE STREQUAL "AUTO" AND _elle_lang_headers_found))

    if(NOT _elle_lang_headers_found)
        message(FATAL_ERROR
            "ELLE_PROB_WITH_LANGUAGE_ENGINE=ON but language headers not found at "
            "${ELLE_LANGUAGE_DIR}/include/elle/Types.hpp")
    endif()

    message(STATUS "elle-probability: language-engine bridge ENABLED "
                   "(headers at ${ELLE_LANGUAGE_DIR})")

    target_sources(elle_probability_core PRIVATE src/Bridge.cpp)
    target_include_directories(elle_probability_core
        PUBLIC
            $<BUILD_INTERFACE:${ELLE_LANGUAGE_DIR}/include>
    )
    target_compile_definitions(elle_probability_core
        PUBLIC ELLE_PROB_HAS_LANGUAGE_BRIDGE=1)

    add_executable(bridge_smoke_demo apps/bridge_smoke_demo.cpp)
    target_link_libraries(bridge_smoke_demo PRIVATE elle_probability_core)

    add_library(elle_probability_host STATIC
        service/ProbabilityHost.cpp
        service/ProbabilityProto.cpp
        ${ELLE_LANGUAGE_DIR}/src/StringUtil.cpp
        ${ELLE_LANGUAGE_DIR}/src/Config.cpp
        ${ELLE_LANGUAGE_DIR}/src/InputNormalizer.cpp
        ${ELLE_LANGUAGE_DIR}/src/PhraseScanner.cpp
        ${ELLE_LANGUAGE_DIR}/src/WordFormResolver.cpp
        ${ELLE_LANGUAGE_DIR}/src/IntegerSequenceBuilder.cpp
        ${ELLE_LANGUAGE_DIR}/src/SenseCandidateResolver.cpp
        ${ELLE_LANGUAGE_DIR}/src/ContextComparator.cpp
        ${ELLE_LANGUAGE_DIR}/src/EmotionalWeightProcessor.cpp
        ${ELLE_LANGUAGE_DIR}/src/SemanticGraphWalker.cpp
        ${ELLE_LANGUAGE_DIR}/src/MeaningObjectBuilder.cpp
        ${ELLE_LANGUAGE_DIR}/src/DebugTrace.cpp
        ${ELLE_LANGUAGE_DIR}/src/InMemoryAccessLayer.cpp
        ${ELLE_LANGUAGE_DIR}/src/Engine.cpp
    )
    target_include_directories(elle_probability_host
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${ELLE_LANGUAGE_DIR}/include>
    )
    target_link_libraries(elle_probability_host
        PUBLIC
            elle_probability_core
    )

    add_executable(prob_host_smoke service/host_smoke_test.cpp)
    target_link_libraries(prob_host_smoke PRIVATE elle_probability_host)

    add_executable(prob_proto_smoke service/proto_smoke_test.cpp)
    target_link_libraries(prob_proto_smoke PRIVATE elle_probability_host)

    target_sources(elle_prob_tests PRIVATE tests/test_bridge.cpp)
else()
    message(STATUS "elle-probability: language-engine bridge disabled "
                   "(headers not found / ELLE_PROB_WITH_LANGUAGE_ENGINE=OFF)")
endif()

install(TARGETS elle_probability_core
    EXPORT elle_probability_targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
