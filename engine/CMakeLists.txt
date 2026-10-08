cmake_minimum_required(VERSION 3.20)
project(chess_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(httplib json)

add_library(chess_core
    src/board.cpp
    src/evaluate.cpp
    src/search.cpp
)
target_include_directories(chess_core PUBLIC src)

add_executable(perft src/perft_main.cpp)
target_link_libraries(perft PRIVATE chess_core)

add_executable(bestmove src/search_main.cpp)
target_link_libraries(bestmove PRIVATE chess_core)

add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE chess_core httplib::httplib nlohmann_json::nlohmann_json)
if (WIN32)
    target_compile_definitions(server PRIVATE _WIN32_WINNT=0x0A00)
endif()

option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(engine_tests
        tests/test_movegen.cpp
        tests/test_search.cpp
    )
    target_link_libraries(engine_tests PRIVATE chess_core GTest::gtest_main)
endif()