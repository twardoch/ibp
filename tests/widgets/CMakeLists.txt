# Widget Library Tests

add_executable(widgets_tests
    test_colorpicker.cpp
    test_slider.cpp
    test_imageviewer.cpp
)

target_link_libraries(widgets_tests
    ibp_test_utils
    ${GTEST_MAIN_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

gtest_discover_tests(widgets_tests)