
add_executable(config_loader_test
    config_loader_test.cpp
    )
target_link_libraries(config_loader_test
    config
    gtest
    )
add_test(
    NAME config_loader_test
    COMMAND $<TARGET_FILE:config_loader_test>
)
