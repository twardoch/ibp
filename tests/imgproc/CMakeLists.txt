# Image Processing Library Tests

# Test executable for imgproc library
add_executable(imgproc_tests
    test_imagefilter.cpp
    test_imagefilterlist.cpp
    test_imagehistogram.cpp
    test_colorconversion.cpp
    test_util.cpp
)

target_link_libraries(imgproc_tests
    ibp_test_utils
    ${GTEST_MAIN_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Register with CTest
gtest_discover_tests(imgproc_tests)