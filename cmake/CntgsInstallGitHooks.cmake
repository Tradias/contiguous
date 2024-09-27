# Copyright 2024 Dennis Hezel
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
# specific language governing permissions and limitations under the License.

find_package(Git)

function(cntgs_create_init_git_hooks_target)
    if(TARGET cntgs-init-git-hooks)
        return()
    endif()

    set(CNTGS_GIT_HOOKS_TARGET_DIR "${CMAKE_SOURCE_DIR}/.git/hooks")
    set(CNTGS_GIT_HOOKS_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/git-hooks/")

    if(NOT EXISTS "${CNTGS_GIT_HOOKS_TARGET_DIR}/pre-commit" OR NOT EXISTS
                                                                "${CNTGS_GIT_HOOKS_TARGET_DIR}/CntgsPreCommit.cmake")
        message(
            AUTHOR_WARNING
                "Initialize clang-format and cmake-format pre-commit hooks by building the CMake target cntgs-init-git-hooks."
        )
    endif()

    find_program(CNTGS_CMAKE_FORMAT_PROGRAM cmake-format)
    find_program(CNTGS_CLANG_FORMAT_PROGRAM clang-format)

    if(NOT CNTGS_CMAKE_FORMAT_PROGRAM OR NOT CNTGS_CLANG_FORMAT_PROGRAM)
        message(
            AUTHOR_WARNING
                "Cannot create init-git-hooks target with\ncmake-format: ${CNTGS_CMAKE_FORMAT_PROGRAM}\nclang-format: ${CNTGS_CLANG_FORMAT_PROGRAM}"
        )
        return()
    endif()

    set(CNTGS_INIT_GIT_HOOKS_SOURCES "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/hooks/pre-commit.in"
                                     "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/hooks/CntgsPreCommit.cmake.in")
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/hooks/pre-commit.in" "${CNTGS_GIT_HOOKS_SOURCE_DIR}/pre-commit"
                   @ONLY NEWLINE_STYLE UNIX)
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/hooks/CntgsPreCommit.cmake.in"
                   "${CNTGS_GIT_HOOKS_SOURCE_DIR}/CntgsPreCommit.cmake" @ONLY NEWLINE_STYLE UNIX)

    set(_cntgs_command_arguments
        "-DGIT_HOOKS_TARGET_DIR=${CNTGS_GIT_HOOKS_TARGET_DIR}" "-DGIT_HOOKS_SOURCE_DIR=${CNTGS_GIT_HOOKS_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CntgsGitHooksInstaller.cmake")
    string(REPLACE ";" " " _cntgs_pretty_command_arguments "${_cntgs_command_arguments}")
    add_custom_target(
        cntgs-init-git-hooks
        DEPENDS ${CNTGS_INIT_GIT_HOOKS_SOURCES}
        SOURCES ${CNTGS_INIT_GIT_HOOKS_SOURCES}
        COMMAND ${CMAKE_COMMAND} ${_cntgs_command_arguments}
        COMMENT "cmake ${_cntgs_pretty_command_arguments}"
        VERBATIM)
endfunction()

if(GIT_FOUND)
    cntgs_create_init_git_hooks_target()
endif()
