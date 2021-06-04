add_subdirectory(codeGen)

# disassemble code-gen
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        COMMAND dumpbin $<TARGET_OBJECTS:cntgs-code-gen-objects> /disasm:nobytes
                /out:${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        DEPENDS cntgs-code-gen-objects $<TARGET_PROPERTY:cntgs-code-gen-objects,SOURCES> cntgs
                $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        COMMAND objdump --disassemble=.text -drwC -M intel -S $<TARGET_OBJECTS:cntgs-code-gen-objects> >
                ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        DEPENDS cntgs-code-gen-objects $<TARGET_PROPERTY:cntgs-code-gen-objects,SOURCES> cntgs
                $<TARGET_PROPERTY:cntgs,CNTGS_SOURCE_FILES>)
endif()

# test utils
add_library(cntgs-test-utils OBJECT)

target_sources(cntgs-test-utils PRIVATE utils/codeGenParser.h utils/codeGenParser.cpp utils/string.h utils/string.cpp)

target_compile_options(cntgs-test-utils PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>)

target_compile_definitions(cntgs-test-utils
                           PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm\"")

target_link_libraries(cntgs-test-utils PRIVATE cntgs)

target_include_directories(cntgs-test-utils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                   $<INSTALL_INTERFACE:include>)

# tests
find_package(doctest REQUIRED)

function(cntgs_add_test _name)
    add_executable(${_name})

    target_sources(${_name} PRIVATE main.cpp ${ARGN})

    target_compile_options(
        ${_name}
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4
                /GR-
                /EHsc->
                $<$<CXX_COMPILER_ID:GNU>:-Wall
                -Wextra
                -pedantic-errors
                -fno-exceptions
                -fno-rtti>)

    target_compile_definitions(${_name}
                               PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm\"")

    target_link_libraries(${_name} PRIVATE cntgs doctest::doctest cntgs-test-utils)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                               $<INSTALL_INTERFACE:include>)
endfunction()

set(CNTGS_TEST_SOURCE_FILES test-contiguous.cpp test-code-gen.cpp ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm)
mark_as_advanced(CNTGS_TEST_SOURCE_FILES)

cntgs_add_test(cntgs-test-cpp17 ${CNTGS_TEST_SOURCE_FILES})

cntgs_add_test(cntgs-test-cpp20 ${CNTGS_TEST_SOURCE_FILES})
target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)

set_source_files_properties(
    test-code-gen.cpp
    PROPERTIES COMPILE_OPTIONS
               "$<$<CXX_COMPILER_ID:MSVC>:/EHs>;$<$<CXX_COMPILER_ID:MSVC>:/GR>;$<$<CXX_COMPILER_ID:GNU>:-fexceptions>")
