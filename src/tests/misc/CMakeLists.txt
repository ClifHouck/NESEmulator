add_executable(console_test
    console_test.cpp
)

target_link_libraries(console_test
    Utility
)

add_test(console_test
    ${CMAKE_CURRENT_BINARY_DIR}/console_test
)
