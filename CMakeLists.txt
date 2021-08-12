cmake_minimum_required(VERSION 3.15)

project(
    contiguous
    VERSION 0.1
    DESCRIPTION "C++ library for storing objects of different types contiguously"
    HOMEPAGE_URL "https://github.com/Tradias/contiguous"
    LANGUAGES CXX)

option(CNTGS_INSTALL on)
option(CNTGS_BUILD_TESTS off)
option(CNTGS_BUILD_DOCUMENTATION off)
option(CNTGS_DISCOVER_TESTS off)
option(CNTGS_TEST_COVERAGE off)
option(CNTGS_ENABLE_DYNAMIC_ANALYSIS off)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_subdirectory(src)

if(CNTGS_BUILD_TESTS)
    find_package(doctest REQUIRED)
    enable_testing()
    include(doctest)
    add_subdirectory(test)
    add_subdirectory(example)
    add_subdirectory(benchmark)
endif()

if(CNTGS_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()
