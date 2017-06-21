# Base64 Test
add_executable(logger_test logger_test.cpp)
target_link_libraries(logger_test
    logger
    spdlog
    gtest
)
add_test(
    NAME logger_test
    COMMAND $<TARGET_FILE:logger_test>
)