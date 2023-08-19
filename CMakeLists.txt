cmake_minimum_required(VERSION 3.8)

if(POLICY CMP0090)
    cmake_policy(SET CMP0090 NEW)
endif()

project(ThreadPool VERSION 1.0.0 LANGUAGES CXX)

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

# Build ThreadPool library
check_symbol_exists(clock_gettime "time.h" HMTHRP_HAVE_CLOCK_GETTIME)

find_package(Threads REQUIRED)

# Installation

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ThreadPool
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev
)

## pkgconfig file
if(UNIX AND NOT APPLE)
    set(THREADPOOL_PKG_EXTRA_LIBS " -lpthread -lrt")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThreadPool.pc.in
    ThreadPool.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/ThreadPool.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

## CMake config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThreadPoolConfig.cmake.in
    ThreadPoolConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadPool
)

write_basic_package_version_file(
    ThreadPoolConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    EXPORT ThreadPoolExport
    NAMESPACE ThreadPool::
    FILE ThreadPoolTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadPool
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/ThreadPoolConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/ThreadPoolConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThreadPool
)

export(PACKAGE ThreadPool)

# Tests
if(HMTHRP_TESTING)
    add_subdirectory(test)
endif()

