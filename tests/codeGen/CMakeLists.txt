include(CntgsCompileOptions)
strip_cmake_cxx_flags_debug(cntgs_previous_cmake_cxx_flags_debug)

# code-gen
add_library(cntgs-code-gen-objects OBJECT)

target_sources(cntgs-code-gen-objects PRIVATE codeGen.cpp)

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

set_property(TARGET cntgs-code-gen-objects PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

target_compile_definitions(cntgs-code-gen-objects PRIVATE NDEBUG)

target_link_libraries(cntgs-code-gen-objects PRIVATE cntgs)

target_include_directories(cntgs-code-gen-objects PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

unset(cntgs_previous_cmake_cxx_flags_debug)
