cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)

# Add cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GetVersion)

# Get version from git tags or file
get_version_from_file(PROJECT_VERSION)

# Get git metadata
get_git_metadata()

# Parse version components
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[a-zA-Z0-9.-]+)?(\\+[a-zA-Z0-9.-]+)?$" VERSION_MATCH "${PROJECT_VERSION}")
if(VERSION_MATCH)
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
    set(PROJECT_VERSION_PRERELEASE ${CMAKE_MATCH_4})
    set(PROJECT_VERSION_BUILD ${CMAKE_MATCH_5})
else()
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
    set(PROJECT_VERSION_PRERELEASE "")
    set(PROJECT_VERSION_BUILD "")
endif()

# Set build timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

project(ibp VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Configure version header with extended metadata
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/ibp/version.h"
    @ONLY
)

# Add version header to include directories
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# Build options
option(IBP_BUILD_TESTS "Build tests" ON)
option(IBP_BUILD_PLUGINS "Build plugins" ON)
option(IBP_BUILD_BENCHMARKS "Build benchmarks" OFF)

# Enable testing if requested
if(IBP_BUILD_TESTS)
    enable_testing()
    
    # Find Google Test
    find_package(GTest REQUIRED)
    include(GoogleTest)
    
    # Add test utilities
    add_subdirectory(tests)
endif()

add_subdirectory(src)
