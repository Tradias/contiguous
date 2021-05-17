add_library(cntgs INTERFACE)
add_library(cntgs::cntgs ALIAS cntgs)

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
