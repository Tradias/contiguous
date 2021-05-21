# cntgs
add_library(cntgs INTERFACE)
add_library(cntgs::cntgs ALIAS cntgs)

unset(CNTGS_SOURCE_FILES)
list(
    APPEND
    CNTGS_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/iterator.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/parameter.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/span.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/typeErasedVector.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/vector.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/array.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/elementLocator.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/forward.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/iterator.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/math.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/memory.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/parameterTraits.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/tuple.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/typeUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/vector.h
    ${CMAKE_CURRENT_LIST_DIR}/cntgs/contiguous/detail/vectorTraits.h)
set_target_properties(cntgs PROPERTIES CNTGS_SOURCE_FILES "${CNTGS_SOURCE_FILES}")

target_compile_features(cntgs INTERFACE cxx_std_17)

target_include_directories(cntgs INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include>)

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
        PATTERN "*.h")
endif()
