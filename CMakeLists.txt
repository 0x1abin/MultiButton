cmake_minimum_required(VERSION 3.10)
project(multi_button
    VERSION 2.0.0
    LANGUAGES C
    DESCRIPTION "Lightweight multi-button state machine library for embedded systems"
)

# ── Library target ───────────────────────────────────────────────────────────
add_library(multi_button
    src/multi_button.c
)

target_include_directories(multi_button
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
)

target_compile_features(multi_button PUBLIC c_std_99)

set_target_properties(multi_button PROPERTIES
    C_STANDARD          99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
)

# Optional: allow users to inject a custom config header via CMake variable.
# Usage: cmake -DBTN_USER_CFG_FILE=\"my_cfg.h\" ..
if(BTN_USER_CFG_FILE)
    target_compile_definitions(multi_button PUBLIC
        BTN_USER_CFG_FILE=${BTN_USER_CFG_FILE}
    )
endif()

# ── Examples (only when built as top-level project) ──────────────────────────
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(MULTI_BUTTON_BUILD_EXAMPLES "Build example programs" ON)

    if(MULTI_BUTTON_BUILD_EXAMPLES)
        foreach(example basic_example advanced_example poll_example)
            add_executable(${example} examples/${example}.c)
            target_link_libraries(${example} PRIVATE multi_button)
        endforeach()
    endif()
endif()

# ── Install rules ────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS multi_button
    EXPORT  multi_button-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT multi_button-targets
    FILE        multi_button-config.cmake
    NAMESPACE   multi_button::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/multi_button
)
