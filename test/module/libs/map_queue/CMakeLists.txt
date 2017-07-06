# DateTime Test
add_executable(map_queue_test
        map_queue_test.cpp
        )
target_link_libraries(map_queue_test
        gtest
        )
add_test(
        NAME map_queue_test
        COMMAND $<TARGET_FILE:map_queue_test>
)
