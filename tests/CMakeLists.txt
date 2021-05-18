find_package(doctest REQUIRED)

function(cntgs_add_test _name)
    add_executable(${_name})

    target_sources(${_name} PRIVATE main.cpp test-contiguous.cpp)

    target_compile_options(${_name} PRIVATE $<$<PLATFORM_ID:Windows>:/W4>)

    target_link_libraries(${_name} PRIVATE cntgs doctest::doctest)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/utils>
                                               $<INSTALL_INTERFACE:include>)
endfunction()

cntgs_add_test(cntgs-test-cpp17)
cntgs_add_test(cntgs-test-cpp20)

target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)

# include(${CMAKE_PREFIX_PATH}/lib/cmake/doctest/doctest.cmake) doctest_discover_tests(cntgs-test ADD_LABELS 0)
