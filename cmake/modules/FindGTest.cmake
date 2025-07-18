# Find Google Test
# This module defines:
#  GTEST_FOUND - system has Google Test
#  GTEST_INCLUDE_DIRS - the Google Test include directories
#  GTEST_LIBRARIES - the libraries needed to use Google Test
#  GTEST_MAIN_LIBRARIES - the libraries needed to use Google Test with main function

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_GTEST QUIET gtest)
    pkg_check_modules(PC_GTEST_MAIN QUIET gtest_main)
endif()

# Try to find Google Test using the standard find_package mechanism first
find_package(GTest QUIET)

if(GTest_FOUND OR GTEST_FOUND)
    # Use the system-installed Google Test
    if(TARGET GTest::gtest)
        set(GTEST_LIBRARIES GTest::gtest)
        set(GTEST_MAIN_LIBRARIES GTest::gtest_main)
        get_target_property(GTEST_INCLUDE_DIRS GTest::gtest INTERFACE_INCLUDE_DIRECTORIES)
    else()
        # Fallback for older CMake versions
        find_path(GTEST_INCLUDE_DIRS
            NAMES gtest/gtest.h
            PATHS ${PC_GTEST_INCLUDE_DIRS}
        )
        
        find_library(GTEST_LIBRARIES
            NAMES gtest
            PATHS ${PC_GTEST_LIBRARY_DIRS}
        )
        
        find_library(GTEST_MAIN_LIBRARIES
            NAMES gtest_main
            PATHS ${PC_GTEST_MAIN_LIBRARY_DIRS}
        )
    endif()
    
    set(GTEST_FOUND TRUE)
else()
    # Download and build Google Test as part of the build process
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
    # Create aliases for consistency
    if(TARGET gtest)
        add_library(GTest::gtest ALIAS gtest)
        add_library(GTest::gtest_main ALIAS gtest_main)
        
        set(GTEST_LIBRARIES gtest)
        set(GTEST_MAIN_LIBRARIES gtest_main)
        set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include)
        set(GTEST_FOUND TRUE)
    else()
        set(GTEST_FOUND FALSE)
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTest
    REQUIRED_VARS GTEST_LIBRARIES GTEST_INCLUDE_DIRS
    VERSION_VAR GTEST_VERSION
)

mark_as_advanced(GTEST_INCLUDE_DIRS GTEST_LIBRARIES GTEST_MAIN_LIBRARIES)