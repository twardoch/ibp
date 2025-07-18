# Miscellaneous Library Tests

add_executable(misc_tests
    test_configuration.cpp
    test_interpolation.cpp
    test_probability.cpp
)

target_link_libraries(misc_tests
    ibp_test_utils
    ${GTEST_MAIN_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

gtest_discover_tests(misc_tests)