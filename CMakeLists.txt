cmake_minimum_required(VERSION 3.1.0)

include(GNUInstallDirs)

# Add cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GetVersion)

# Get version from file
get_version_from_file(PROJECT_VERSION)

project(ibp VERSION ${PROJECT_VERSION})

# Configure version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/ibp/version.h"
    @ONLY
)

# Add version header to include directories
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

add_subdirectory(src)
