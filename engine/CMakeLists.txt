cmake_minimum_required(VERSION 3.20)
project(chess_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

add_library(chess_core
    src/board.cpp
)
target_include_directories(chess_core PUBLIC src)

add_executable(perft src/perft_main.cpp)
target_link_libraries(perft PRIVATE chess_core)

option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(engine_tests tests/test_movegen.cpp)
    target_link_libraries(engine_tests PRIVATE chess_core GTest::gtest_main)
endif()