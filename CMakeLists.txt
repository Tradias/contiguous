cmake_minimum_required(VERSION 3.15)

project(
    contiguous
    VERSION 0.1
    DESCRIPTION "Library for storing structs in contiguous memory"
    HOMEPAGE_URL https://github.com/Tradias/contiguous
    LANGUAGES CXX)

option(CNTGS_NO_INSTALL on)
option(CNTGS_BUILD_TESTS off)
option(CNTGS_BUILD_DOCUMENTATION off)
option(CNTGS_DISCOVER_TESTS off)
option(CNTGS_TEST_COVERAGE off)
set(CNTGS_TEST_COVERAGE_OUTPUT_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/test-coverage"
    CACHE PATH "Directory for test coverage reports")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_subdirectory(src)

if(CNTGS_BUILD_TESTS)
    add_subdirectory(tests)
    add_subdirectory(benchmark)
endif()

if(CNTGS_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()
