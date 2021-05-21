function(strip_cmake_cxx_flags_debug _previous_options)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(cntgs_previous_cmake_cxx_flags_debug ${CMAKE_CXX_FLAGS_DEBUG})
        set(cntgs_test_code_gen_compile_options "${CMAKE_CXX_FLAGS_DEBUG}")
        set(CMAKE_CXX_FLAGS_DEBUG
            ""
            PARENT_SCOPE)
        string(REPLACE " " ";" cntgs_previous_cmake_cxx_flags_debug "${cntgs_previous_cmake_cxx_flags_debug}")
        set(${_previous_options}
            "${cntgs_previous_cmake_cxx_flags_debug}"
            PARENT_SCOPE)
    endif()
endfunction()
