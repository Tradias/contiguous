find_program(CNTGS_STANDARDESE_TOOL standardese)

add_custom_target(
    cntgs-generate-documentation
    COMMAND
        ${CNTGS_STANDARDESE_TOOL} "--output.prefix=${CMAKE_CURRENT_BINARY_DIR}/documentation/"
        "--input.blacklist_dir=detail" "--compilation.include_dir=${CMAKE_SOURCE_DIR}/src"
        "--compilation.standard=c++17" "${CMAKE_SOURCE_DIR}/src/cntgs/contiguous"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating documentation")
