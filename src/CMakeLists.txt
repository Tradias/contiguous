# cntgs
add_library(cntgs INTERFACE)
add_library(cntgs::cntgs ALIAS cntgs)

unset(CNTGS_SOURCE_FILES)
list(
    APPEND
    CNTGS_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/element.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/iterator.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/parameter.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/reference.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/referenceQualifier.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/span.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/typeErasedVector.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/vector.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/algorithm.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/array.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/attributes.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/elementLocator.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/elementTraits.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/forward.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/iteratorUtils.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/memory.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/parameterListTraits.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/parameterTraits.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/parameterType.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/range.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/sizeGetter.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/tuple.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/typeUtils.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/utility.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/vectorTraits.hpp")
set_target_properties(cntgs PROPERTIES CNTGS_SOURCE_FILES "${CNTGS_SOURCE_FILES}")

target_compile_features(cntgs INTERFACE cxx_std_17)

target_include_directories(cntgs INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include>)

# cntgs objects
if(CNTGS_BUILD_TESTS)
    add_library(cntgs-objects OBJECT)

    target_sources(cntgs-objects PRIVATE cntgs.cpp)

    target_link_libraries(cntgs-objects PUBLIC cntgs)

    target_compile_options(
        cntgs-objects
        PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/W4
               /GR-
               /EHsc->
               $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall
               -Wextra
               -pedantic-errors
               -fno-exceptions
               -fno-rtti>)

    # amalgamate
    find_package(Python COMPONENTS Interpreter)
    if(TARGET "Python::Interpreter")
        set(CNTGS_AMALGAMATE_PY
            ""
            CACHE FILEPATH "Path to amalgamate.py from https://github.com/edlund/amalgamate")
        if(CNTGS_AMALGAMATE_PY)
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_LIST_DIR}/cntgs.hpp"
                COMMAND Python::Interpreter ARGS "${CNTGS_AMALGAMATE_PY}" "-c"
                        "${CMAKE_CURRENT_LIST_DIR}/amalgamate.json" "-s" "${CMAKE_CURRENT_LIST_DIR}"
                DEPENDS "${CNTGS_SOURCE_FILES}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
            add_custom_target(
                cntgs-amalgamate
                DEPENDS "${CMAKE_CURRENT_LIST_DIR}/cntgs.hpp"
                SOURCES "${CMAKE_CURRENT_LIST_DIR}/amalgamate.json")

            target_sources(cntgs-objects PRIVATE cntgs.hpp)
        endif()
    endif()
endif()

unset(CNTGS_SOURCE_FILES)

# installation
if(NOT CNTGS_NO_INSTALL)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/generated/cntgsConfigVersion.cmake" ARCH_INDEPENDENT
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/cntgsConfigVersion.cmake" DESTINATION share/cntgs)

    include(GNUInstallDirs)
    install(
        TARGETS cntgs
        EXPORT CNTGS_EXPORT_TARGETS
        PUBLIC_HEADER)

    install(
        EXPORT CNTGS_EXPORT_TARGETS
        NAMESPACE cntgs::
        FILE cntgsConfig.cmake
        DESTINATION share/cntgs)

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cntgs"
        TYPE INCLUDE
        FILES_MATCHING
        PATTERN "*.hpp")
endif()
