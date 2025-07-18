# C++ Tests for Image Batch Processor

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/include)

# Test utilities library
add_library(ibp_test_utils STATIC
    test_utils.cpp
    test_utils.h
)

target_link_libraries(ibp_test_utils
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Core library tests
add_subdirectory(imgproc)
add_subdirectory(misc)
add_subdirectory(widgets)

# Integration tests
add_executable(integration_tests
    integration_tests.cpp
)

target_link_libraries(integration_tests
    ibp_test_utils
    ${GTEST_MAIN_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Register tests with CTest
gtest_discover_tests(integration_tests)