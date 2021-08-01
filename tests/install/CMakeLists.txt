add_test(
    NAME cntgs-test-install
    COMMAND ${CMAKE_COMMAND} "-DPWD=${CMAKE_CURRENT_BINARY_DIR}/temp-test-install" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
            "-DUSER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/user" -P "${CMAKE_CURRENT_LIST_DIR}/Test.cmake")

add_test(
    NAME cntgs-test-subdirectory
    COMMAND
        ${CMAKE_COMMAND} "-DPWD=${CMAKE_CURRENT_BINARY_DIR}/temp-test-subdirectory" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
        "-DUSER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/user" "-DBINARY_DIR=${CMAKE_BINARY_DIR}"
        "-DADD_SUBDIRECTORY=${CMAKE_SOURCE_DIR}" -P "${CMAKE_CURRENT_LIST_DIR}/Test.cmake")
