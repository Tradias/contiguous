add_subdirectory(codeGen)

# disassemble code-gen
function(cntgs_disassemble_code_gen _cntgs_target)
    set(cntgs_disassembly_output_file ${CMAKE_CURRENT_BINARY_DIR}/${_cntgs_target}.asm)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        add_custom_command(
            OUTPUT ${cntgs_disassembly_output_file}
            COMMAND dumpbin $<TARGET_OBJECTS:${_cntgs_target}> /disasm:nobytes /out:${cntgs_disassembly_output_file}
            DEPENDS ${_cntgs_target} $<TARGET_PROPERTY:${_cntgs_target},SOURCES> cntgs
                    $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_custom_command(
            OUTPUT ${cntgs_disassembly_output_file}
            COMMAND objdump --disassemble=.text -drwC -M intel -S $<TARGET_OBJECTS:${_cntgs_target}> >
                    ${cntgs_disassembly_output_file}
            DEPENDS ${_cntgs_target} $<TARGET_PROPERTY:${_cntgs_target},SOURCES> cntgs
                    $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
    endif()
endfunction()

cntgs_disassemble_code_gen(cntgs-code-gen-objects-cpp17)
cntgs_disassemble_code_gen(cntgs-code-gen-objects-cpp20)

add_library(cntgs-test-utils OBJECT)

target_sources(cntgs-test-utils PRIVATE utils/codeGenParser.h utils/codeGenParser.cpp utils/string.h utils/string.cpp)

target_compile_options(cntgs-test-utils PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra
                                                -pedantic-errors>)

target_link_libraries(cntgs-test-utils PRIVATE cntgs)

target_include_directories(cntgs-test-utils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                   $<INSTALL_INTERFACE:include>)

# tests
find_package(doctest REQUIRED)

function(cntgs_add_test _cntgs_name _cntgs_disassembly_file)
    add_executable(${_cntgs_name})

    target_sources(${_cntgs_name} PRIVATE main.cpp ${_cntgs_disassembly_file}
                                          $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES> ${ARGN})

    target_compile_options(
        ${_cntgs_name}
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4
                /GR-
                /EHsc->
                $<$<CXX_COMPILER_ID:GNU>:-Wall
                -Wextra
                -pedantic-errors
                -fno-exceptions
                -fno-rtti>)

    target_compile_definitions(${_cntgs_name} PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${_cntgs_disassembly_file}\"")

    target_link_libraries(${_cntgs_name} PRIVATE cntgs doctest::doctest cntgs-test-utils)

    target_include_directories(${_cntgs_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                     $<INSTALL_INTERFACE:include>)
endfunction()

set(CNTGS_TEST_SOURCE_FILES test-contiguous.cpp test-code-gen.cpp)

cntgs_add_test(cntgs-test-cpp17 ${CMAKE_CURRENT_BINARY_DIR}/cntgs-code-gen-objects-cpp17.asm ${CNTGS_TEST_SOURCE_FILES})

cntgs_add_test(cntgs-test-cpp20 ${CMAKE_CURRENT_BINARY_DIR}/cntgs-code-gen-objects-cpp20.asm ${CNTGS_TEST_SOURCE_FILES})
target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)

unset(CNTGS_TEST_SOURCE_FILES)

set_source_files_properties(
    test-code-gen.cpp
    PROPERTIES COMPILE_OPTIONS
               "$<$<CXX_COMPILER_ID:MSVC>:/EHs>;$<$<CXX_COMPILER_ID:MSVC>:/GR>;$<$<CXX_COMPILER_ID:GNU>:-fexceptions>")
