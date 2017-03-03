set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)
set(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

add_executable(config_test
    config_test.cpp
)

target_link_libraries(config_test
   gtest
   json   
)

add_test(
    NAME config_test
    COMMAND $<TARGET_FILE:config_test>
)


add_executable(http_client_test
    http_client_test.cpp
)
target_link_libraries(http_client_test
    http_client
    gtest
    logger
)
add_test(
    NAME http_client_test
    COMMAND $<TARGET_FILE:config_test>
)
