function(cntgs_add_coverage_flags _cntgs_target)
    target_compile_options(${_cntgs_target} PRIVATE --coverage)
    target_link_libraries(${_cntgs_target} PRIVATE gcov)
endfunction()

function(cntgs_coverage_report_for_target _cntgs_target _cntgs_source)
    get_filename_component(_cntgs_source_name "${_cntgs_source}" NAME)
    add_custom_target(
        ${_cntgs_target}-coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CNTGS_TEST_COVERAGE_OUTPUT_DIR}
        DEPENDS ${_cntgs_target})
    add_custom_command(
        TARGET ${_cntgs_target}-coverage
        COMMAND
            gcov --relative-only --demangled-names -o
            $<TARGET_FILE_DIR:${_cntgs_target}>/CMakeFiles/${_cntgs_target}.dir/${_cntgs_source_name}.gcda
            "${_cntgs_source}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()
