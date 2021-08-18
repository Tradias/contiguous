function(cntgs_add_coverage_flags _cntgs_target)
    target_compile_options(${_cntgs_target} PRIVATE --coverage)
    target_link_libraries(${_cntgs_target} PRIVATE gcov)
endfunction()

function(cntgs_coverage_report_for_target _cntgs_target)
    find_program(CNTGS_GCOV_PROGRAM gcov)
    add_custom_target(
        ${_cntgs_target}-coverage
        DEPENDS ${_cntgs_target}
        COMMAND "${CNTGS_GCOV_PROGRAM}" --relative-only --demangled-names --preserve-paths
                $<TARGET_OBJECTS:${_cntgs_target}>
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMAND_EXPAND_LISTS)
endfunction()
