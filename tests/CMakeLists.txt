find_package(doctest REQUIRED)

function(cntgs_add_test _name)
    add_executable(${_name})

    target_sources(${_name} PRIVATE main.cpp test-contiguous.cpp)

    target_compile_options(${_name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /GR- /EHs->)

    target_link_libraries(${_name} PRIVATE cntgs doctest::doctest)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/utils>
                                               $<INSTALL_INTERFACE:include>)
endfunction()

cntgs_add_test(cntgs-test-cpp17)
cntgs_add_test(cntgs-test-cpp20)

target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)

# code-gen
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        COMMAND ${CMAKE_CXX_COMPILER} /FA -I${CMAKE_SOURCE_DIR}/src /std:c++17 /O2 /GR- /EHs-
                -I${CMAKE_CURRENT_SOURCE_DIR}/utils -c ${CMAKE_CURRENT_SOURCE_DIR}/code-gen.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/code-gen.cpp)

    add_custom_target(cntgs-code-gen DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm)
endif()
