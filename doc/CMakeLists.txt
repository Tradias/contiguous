find_program(CNTGS_STANDARDESE_TOOL standardese)

add_custom_target(
    cntgs-generate-documentation
    COMMAND
        ${CNTGS_STANDARDESE_TOOL} "--output.prefix=${CMAKE_CURRENT_BINARY_DIR}/documentation/"
        "--compilation.commands_dir=${CMAKE_BINARY_DIR}" "--input.blacklist_dir=detail"
        "${CMAKE_SOURCE_DIR}/src/cntgs/contiguous"
    COMMENT "Generating documentation")
