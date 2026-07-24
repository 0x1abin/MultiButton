cmake_minimum_required(VERSION 3.10)
project(MultiButton VERSION 1.1.1 LANGUAGES C)

# Library
add_library(multibutton multi_button.c)
target_include_directories(multibutton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(multibutton PUBLIC c_std_99)

# Examples
option(MULTIBUTTON_BUILD_EXAMPLES "Build example programs" OFF)
if(MULTIBUTTON_BUILD_EXAMPLES)
    add_executable(basic_example examples/basic_example.c)
    target_link_libraries(basic_example multibutton)

    add_executable(advanced_example examples/advanced_example.c)
    target_link_libraries(advanced_example multibutton)

    add_executable(poll_example examples/poll_example.c)
    target_link_libraries(poll_example multibutton)
endif()

# Tests
option(MULTIBUTTON_BUILD_TESTS "Build unit tests" OFF)
if(MULTIBUTTON_BUILD_TESTS)
    enable_testing()
    add_executable(test_button tests/test_button.c)
    target_link_libraries(test_button multibutton)
    add_test(NAME button_tests COMMAND test_button)

    add_executable(test_button_no_double
        multi_button.c
        tests/test_button_no_double.c
    )
    target_compile_definitions(test_button_no_double
        PRIVATE MULTIBUTTON_ENABLE_DOUBLE_CLICK=0
    )
    target_include_directories(test_button_no_double
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(NAME button_no_double_tests COMMAND test_button_no_double)
endif()
