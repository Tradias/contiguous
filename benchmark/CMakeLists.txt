find_package(benchmark)

function(cntgs_add_benchmark _name)
    add_executable(${_name})

    target_sources(
        ${_name}
        PRIVATE main.cpp
                benchmark.cpp
                nearestNeighbor/benchmark.cpp
                nearestNeighbor/distance.hpp
                nearestNeighbor/graph.hpp
                nearestNeighbor/load.hpp
                nearestNeighbor/repository.hpp
                nearestNeighbor/search.hpp)

    target_compile_options(${_name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
                                            -Wextra -pedantic-errors -msse4.1>)

    target_compile_definitions(${_name} PRIVATE "CNTGS_BENCHMARK_GRAPH_DATA_DIR=\"${CNTGS_BENCHMARK_GRAPH_DATA_DIR}\"")

    target_link_libraries(${_name} PRIVATE cntgs cntgs-test-utils benchmark::benchmark)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                               $<INSTALL_INTERFACE:include>)
endfunction()

cntgs_add_benchmark(cntgs-benchmark-cpp17 ${CNTGS_TEST_SOURCE_FILES})

cntgs_add_benchmark(cntgs-benchmark-cpp20 ${CNTGS_TEST_SOURCE_FILES})
target_compile_features(cntgs-benchmark-cpp20 PRIVATE cxx_std_20)
