function(run_process _working_dir)
    execute_process(
        COMMAND ${ARGN}
        WORKING_DIRECTORY "${_working_dir}"
        TIMEOUT 120
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _output
        ERROR_VARIABLE _output)

    if(NOT ${_result} EQUAL 0)
        message(FATAL_ERROR "Command \"${ARGN}\" failed with: ${_result}\n${_output}")
    endif()
endfunction()

file(REMOVE_RECURSE "${PWD}/.")
file(MAKE_DIRECTORY "${PWD}/test-build")

if(ADD_SUBDIRECTORY)
    get_filename_component(_source_dir_name "${SOURCE_DIR}" NAME)
    get_filename_component(_binary_dir_name "${BINARY_DIR}" NAME)
    file(
        COPY "${SOURCE_DIR}"
        DESTINATION "${PWD}"
        REGEX ".*${_binary_dir_name}.*" EXCLUDE)

    file(COPY "${USER_SOURCE_DIR}/." DESTINATION "${PWD}")

    # build user project
    run_process("${PWD}/test-build" "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_PREFIX=${PWD}/test-install"
                "-DADD_SUBDIRECTORY=${PWD}/${_source_dir_name}" "${PWD}")
else()
    file(MAKE_DIRECTORY "${PWD}/build")

    # build cntgs
    run_process("${PWD}/build" "${CMAKE_COMMAND}" "-DCNTGS_INSTALL=on" "-DCMAKE_INSTALL_PREFIX=${PWD}/install"
                "${SOURCE_DIR}")

    # install cntgs
    run_process("${PWD}/build" "${CMAKE_COMMAND}" --build . --target install)

    # build user project
    run_process("${PWD}/test-build" "${CMAKE_COMMAND}" "-DCMAKE_PREFIX_PATH=${PWD}/install"
                "-DCMAKE_INSTALL_PREFIX=${PWD}/test-install" "${USER_SOURCE_DIR}")
endif()

# install user project
run_process("${PWD}/test-build" "${CMAKE_COMMAND}" --build . --target install)

if(EXISTS "${PWD}/test-install/include")
    message(FATAL_ERROR "Fail: cntgs was installed along with user project")
endif()

# run user project
run_process("${PWD}/test-install" "${PWD}/test-install/bin/app${CMAKE_EXECUTABLE_SUFFIX}")
