find_package(doctest REQUIRED)

add_executable(cntgs-test)

target_sources(cntgs-test PRIVATE main.cpp test-contiguous.cpp)

target_compile_options(cntgs-test PRIVATE $<$<PLATFORM_ID:Windows>:/W4>)

target_link_libraries(cntgs-test PRIVATE cntgs doctest::doctest)

target_include_directories(cntgs-test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/utils>
                                             $<INSTALL_INTERFACE:include>)

# include( ${CMAKE_CURRENT_LIST_DIR}/../third-party/doctest/lib/cmake/doctest/doctest.cmake )
# doctest_discover_tests(test ADD_LABELS 0)
