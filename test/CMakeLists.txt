add_subdirectory(codeGen)
add_subdirectory(install)

# disassemble code-gen
function(cntgs_disassemble_code_gen _cntgs_target)
    set(cntgs_disassembly_output_file "${CMAKE_CURRENT_BINARY_DIR}/${_cntgs_target}.asm")
    if(WIN32)
        find_program(CNTGS_DUMPBIN_PROGRAM NAMES "dumpbin" "dumpbin.exe")
        add_custom_command(
            OUTPUT ${cntgs_disassembly_output_file}
            COMMAND ${CNTGS_DUMPBIN_PROGRAM} $<TARGET_OBJECTS:${_cntgs_target}> /disasm:nobytes
                    /out:${cntgs_disassembly_output_file}
            DEPENDS ${_cntgs_target} $<TARGET_PROPERTY:${_cntgs_target},SOURCES> cntgs
                    $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
    else()
        add_custom_command(
            OUTPUT ${cntgs_disassembly_output_file}
            COMMAND objdump --disassemble=.text -drwC -M intel $<TARGET_OBJECTS:${_cntgs_target}> >
                    ${cntgs_disassembly_output_file}
            DEPENDS ${_cntgs_target} $<TARGET_PROPERTY:${_cntgs_target},SOURCES> cntgs
                    $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
    endif()
endfunction()

cntgs_disassemble_code_gen(cntgs-code-gen-objects-cpp17)
cntgs_disassemble_code_gen(cntgs-code-gen-objects-cpp20)

add_library(cntgs-test-utils OBJECT)

target_sources(
    cntgs-test-utils
    PRIVATE "utils/codeGenParser.hpp"
            "utils/codeGenParser.cpp"
            "utils/check.hpp"
            "utils/functional.hpp"
            "utils/range.hpp"
            "utils/string.hpp"
            "utils/string.cpp"
            "utils/typeTraits.hpp")

target_compile_options(cntgs-test-utils PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
                                                -Wextra -pedantic-errors>)

target_link_libraries(cntgs-test-utils PRIVATE cntgs)

target_include_directories(cntgs-test-utils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                   $<INSTALL_INTERFACE:include>)

# tests
if(CNTGS_TEST_COVERAGE)
    include(CntgsCoverage)
endif()

function(cntgs_add_test _cntgs_name _cntgs_disassembly_file)
    add_executable(${_cntgs_name})

    target_sources(${_cntgs_name} PRIVATE main.cpp ${_cntgs_disassembly_file}
                                          $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES> ${ARGN})

    if(CNTGS_TEST_COVERAGE)
        cntgs_add_coverage_flags(${_cntgs_name})
    endif()

    target_compile_definitions(${_cntgs_name} PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${_cntgs_disassembly_file}\"")

    target_link_libraries(${_cntgs_name} PRIVATE cntgs-objects doctest::doctest cntgs-test-utils)

    target_include_directories(${_cntgs_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                     $<INSTALL_INTERFACE:include>)

    target_precompile_headers(${_cntgs_name} PRIVATE "utils/precompiledHeader.hpp")
endfunction()

set(CNTGS_TEST_SOURCE_FILES "test-contiguous.cpp" "test-code-gen.cpp")

cntgs_add_test(cntgs-test-cpp17 "${CMAKE_CURRENT_BINARY_DIR}/cntgs-code-gen-objects-cpp17.asm"
               ${CNTGS_TEST_SOURCE_FILES})
target_compile_definitions(cntgs-test-cpp17 PRIVATE "CNTGS_TEST_CPP_VERSION=\"C++17\"")

cntgs_add_test(cntgs-test-cpp20 "${CMAKE_CURRENT_BINARY_DIR}/cntgs-code-gen-objects-cpp20.asm"
               ${CNTGS_TEST_SOURCE_FILES})
target_compile_definitions(cntgs-test-cpp20 PRIVATE "CNTGS_TEST_CPP_VERSION=\"C++20\"")
target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)

unset(CNTGS_TEST_SOURCE_FILES)

set_source_files_properties(
    test-code-gen.cpp
    PROPERTIES
        COMPILE_OPTIONS
        "$<$<CXX_COMPILER_ID:MSVC>:/EHs>;$<$<CXX_COMPILER_ID:MSVC>:/GR>;$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fexceptions>"
        SKIP_PRECOMPILE_HEADERS on)

if(CNTGS_DISCOVER_TESTS)
    doctest_discover_tests(
        cntgs-test-cpp17 # workaround for regression in doctest 2.4.6
        #
        # if given arguments:
        #
        # "EQUAL" "1"
        #
        # Unknown arguments specified
        ADD_LABELS
        0)
    doctest_discover_tests(cntgs-test-cpp20 ADD_LABELS 0)
endif()

if(CNTGS_TEST_COVERAGE)
    cntgs_coverage_report_for_target(cntgs-test-cpp17 ${CMAKE_CURRENT_SOURCE_DIR}/test-contiguous.cpp)
endif()
