cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

if(POLICY CMP0090)
    cmake_policy(SET CMP0090 NEW)
endif()

project(Leopard VERSION 1.0.0 LANGUAGES CXX)

option(HMTHRP_TESTING "Enable testing" OFF)
## option(HMTHRP_EXAMPLES "Build Examples" OFF)
## option(HMTHRP_BENCHMARKS "Build Benchmarks" OFF)

if(HMTHRP_TESTING)
    enable_testing()
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

include(HMTHRPTargetDataFiles)

# Build Leopard library
check_symbol_exists(clock_gettime "time.h" HMTHRP_HAVE_CLOCK_GETTIME)

find_package(Threads REQUIRED)

# Installation

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/Leopard
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev
)

## pkgconfig file
if(UNIX AND NOT APPLE)
    set(THREADPOOL_PKG_EXTRA_LIBS " -lpthread -lrt")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Leopard.pc.in
    Leopard.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/Leopard.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

## CMake config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LeopardConfig.cmake.in
    LeopardConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Leopard
)

write_basic_package_version_file(
    LeopardConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    EXPORT LeopardExport
    NAMESPACE Leopard::
    FILE LeopardTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Leopard
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/LeopardConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/LeopardConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Leopard
)

export(PACKAGE Leopard)

# Tests
if(HMTHRP_TESTING)
    add_subdirectory(test)
endif()

