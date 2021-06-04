function(cntgs_add_benchmark _name)
    add_executable(${_name})

    target_sources(${_name} PRIVATE benchmark.cpp)

    target_compile_options(${_name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:GNU>:-Wall
                                                   -Wextra -pedantic-errors>)

    target_link_libraries(${_name} PRIVATE cntgs cntgs-test-utils)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                      $<INSTALL_INTERFACE:include>)
endfunction()

cntgs_add_benchmark(cntgs-benchmark-cpp17 ${CNTGS_TEST_SOURCE_FILES})

cntgs_add_benchmark(cntgs-benchmark-cpp20 ${CNTGS_TEST_SOURCE_FILES})
target_compile_features(cntgs-benchmark-cpp20 PRIVATE cxx_std_20)
