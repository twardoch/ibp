# FindFreeImage.cmake
# Locate FreeImage library
#
# This module defines:
#   FreeImage_FOUND - System has FreeImage
#   FreeImage_INCLUDE_DIRS - FreeImage include directories
#   FreeImage_LIBRARIES - Libraries needed to use FreeImage

include(FindPackageHandleStandardArgs)

# Find include directory
find_path(FreeImage_INCLUDE_DIR
NAMES FreeImage.h
PATHS
    ${FreeImage_DIR}
    $ENV{FreeImage_DIR}
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    "C:/Program Files/FreeImage/include"
    "C:/Program Files (x86)/FreeImage/include"
)

# Determine library prefix and suffix based on platform
if(WIN32)
    set(FreeImage_LIBRARY_NAMES FreeImage)
else()
    set(FreeImage_LIBRARY_NAMES freeimage)
endif()

# Find library
find_library(FreeImage_LIBRARY
NAMES ${FreeImage_LIBRARY_NAMES}
PATHS
    ${FreeImage_DIR}
    $ENV{FreeImage_DIR}
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    "C:/Program Files/FreeImage/lib"
    "C:/Program Files (x86)/FreeImage/lib"
)

# Set output variables
set(FreeImage_INCLUDE_DIRS ${FreeImage_INCLUDE_DIR})
set(FreeImage_LIBRARIES ${FreeImage_LIBRARY})

# Handle REQUIRED and QUIET arguments
# Set FreeImage_FOUND
find_package_handle_standard_args(FreeImage
DEFAULT_MSG
FreeImage_LIBRARY
FreeImage_INCLUDE_DIR
)

mark_as_advanced(
FreeImage_INCLUDE_DIR
FreeImage_LIBRARY
)
