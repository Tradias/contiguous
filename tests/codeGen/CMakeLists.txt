include(CntgsCompileOptions)
strip_cmake_cxx_flags_debug(cntgs_previous_cmake_cxx_flags_debug)

# code-gen
function(cntgs_add_code_gen _cntgs_name)
    add_library(${_cntgs_name} OBJECT)

    target_sources(${_cntgs_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/codeGen.cpp
                                          ${CMAKE_CURRENT_SOURCE_DIR}/reference.hpp)

    target_compile_options(
        ${_cntgs_name}
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/O2
                /Ob2
                /GR-
                /EHsc-
                /Zi
                /DNDEBUG>
                $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3
                -g
                -fno-exceptions
                -fno-rtti>)

    set_property(TARGET ${_cntgs_name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

    target_compile_definitions(${_cntgs_name} PRIVATE NDEBUG)

    target_link_libraries(${_cntgs_name} PRIVATE cntgs)

    target_include_directories(${_cntgs_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

cntgs_add_code_gen(cntgs-code-gen-objects-cpp17)
cntgs_add_code_gen(cntgs-code-gen-objects-cpp20)
target_compile_features(cntgs-code-gen-objects-cpp20 PRIVATE cxx_std_20)

unset(cntgs_previous_cmake_cxx_flags_debug)
