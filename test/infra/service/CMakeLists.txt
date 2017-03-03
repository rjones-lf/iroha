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
