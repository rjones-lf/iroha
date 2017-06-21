# DateTime Test
add_executable(time_test
        time_test.cpp
        )
target_link_libraries(time_test
        time64
        gtest
        )
add_test(
        NAME time_test
        COMMAND $<TARGET_FILE:time_test>
)
