find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(FreeImage REQUIRED)
add_library(
    ibp.imgproc
    SHARED
    colorconversion.cpp
    freeimage.cpp
    imagefilterlist.cpp
    lut01.cpp
    lut02.cpp
    lut03.cpp
    util.cpp
    pixelblending.cpp
    intensitymapping.cpp
    thresholding.cpp
    imagehistogram.cpp
    # Headers should be exposed via target_include_directories, not listed in add_library
)

# Add current source dir for its own headers, and potentially for others if they are public API.
# If these headers are only for this lib's .cpp files, use PRIVATE.
# If they are part of the public API of this lib, use PUBLIC or INTERFACE.
# For now, assuming they might be part of a public API for other local libs/exe.
target_include_directories(ibp.imgproc
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # For its own headers if they form a public API
    ${FreeImage_INCLUDE_DIRS}
)

target_link_libraries(
    ibp.imgproc
    PUBLIC
    ibp.misc
    ibp.plugins
    Qt5::Widgets
    opencv_core
    opencv_imgproc
    freeimage
)

set_target_properties(
    ibp.imgproc
    PROPERTIES
    OUTPUT_NAME ibp.imgproc
    VERSION 0.1.0
    AUTOMOC ON
    RUNTIME_OUTPUT_DIRECTORY ${IBP_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${IBP_OUTPUT_DIRECTORY}
)

install(TARGETS ibp.imgproc)
