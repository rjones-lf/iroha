########################################################################################
# Executable executor_test
########################################################################################
add_executable(executor_test executor_test.cpp)
target_link_libraries(executor_test
    Repository
    executor
    gtest
)
add_test(
    NAME executor_test
    COMMAND $<TARGET_FILE:executor_test>
)