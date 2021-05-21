include(CntgsCompileOptions)
strip_cmake_cxx_flags_debug(cntgs_previous_cmake_cxx_flags_debug)

# code-gen
add_library(cntgs-code-gen-objects OBJECT)

target_sources(cntgs-code-gen-objects PRIVATE code-gen.cpp)

target_compile_options(
    cntgs-code-gen-objects
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/O2
            /Ob2
            /GR-
            /EHsc-
            /Zi
            /DNDEBUG>
            $<$<CXX_COMPILER_ID:GNU>:-O3
            -g
            -fno-exceptions
            -fno-rtti>)

target_compile_definitions(cntgs-code-gen-objects PRIVATE NDEBUG)

target_link_libraries(cntgs-code-gen-objects PRIVATE cntgs)

# disassemble code-gen
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        COMMAND dumpbin $<TARGET_OBJECTS:cntgs-code-gen-objects> /disasm:nobytes
                /out:${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        DEPENDS cntgs-code-gen-objects ${CMAKE_CURRENT_SOURCE_DIR}/code-gen.cpp)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm
        COMMAND objdump -drwC -M intel -S $<TARGET_OBJECTS:cntgs-code-gen-objects>
        DEPENDS cntgs-code-gen-objects ${CMAKE_CURRENT_SOURCE_DIR}/code-gen.cpp)
endif()

# code-gen parser
add_library(cntgs-code-gen-parser OBJECT)

target_sources(cntgs-code-gen-parser PRIVATE utils/codeGenParser.h utils/codeGenParser.cpp)

target_compile_options(
    cntgs-code-gen-parser PRIVATE $<$<CONFIG:Debug>:${cntgs_previous_cmake_cxx_flags_debug}>
                                  $<$<CXX_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>)

target_compile_definitions(cntgs-code-gen-parser
                           PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm\"")

target_link_libraries(cntgs-code-gen-parser PRIVATE cntgs)

target_include_directories(cntgs-code-gen-parser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# tests
find_package(doctest REQUIRED)

function(cntgs_add_test _name)
    add_executable(${_name})

    target_sources(${_name} PRIVATE main.cpp ${ARGN})

    target_compile_options(
        ${_name}
        PRIVATE $<$<CONFIG:Debug>:${cntgs_previous_cmake_cxx_flags_debug}>
                $<$<CXX_COMPILER_ID:MSVC>:/W4
                /GR-
                /EHs->
                $<$<CXX_COMPILER_ID:GNU>:-Wall
                -Wextra
                -fno-exceptions
                -fno-rtti>)

    target_compile_definitions(${_name}
                               PRIVATE "CNTGS_CODE_GEN_DISASSEMBLY_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm\"")

    target_link_libraries(${_name} PRIVATE cntgs doctest::doctest cntgs-code-gen-parser)

    target_include_directories(${_name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                               $<INSTALL_INTERFACE:include>)
endfunction()

set(CNTGS_TEST_SOURCE_FILES test-contiguous.cpp test-code-gen.cpp ${CMAKE_CURRENT_BINARY_DIR}/code-gen.asm)
mark_as_advanced(CNTGS_TEST_SOURCE_FILES)

cntgs_add_test(cntgs-test-cpp17 ${CNTGS_TEST_SOURCE_FILES})

cntgs_add_test(cntgs-test-cpp20 ${CNTGS_TEST_SOURCE_FILES})
target_compile_features(cntgs-test-cpp20 PRIVATE cxx_std_20)
