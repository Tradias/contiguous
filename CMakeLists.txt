cmake_minimum_required(VERSION 3.15)

project(
    contiguous
    VERSION 0.1
    DESCRIPTION "Library for storing structs in contiguous memory"
    LANGUAGES CXX)

option(CNTGS_NO_INSTALL on)
option(CNTGS_BUILD_TESTS off)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

add_subdirectory(src)

if(CNTGS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
