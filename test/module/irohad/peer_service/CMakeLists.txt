# SelfState Test
add_executable(self_state_test
        self_state_test.cpp
        )
target_link_libraries(self_state_test
        peer_service
        gtest
        )
add_test(
        NAME self_state_test
        COMMAND $<TARGET_FILE:self_state_test>
)


# MonitorState Test
add_executable(monitor_test
        monitor_test
        )
target_link_libraries(monitor_test
        peer_service
        gtest
        )
add_test(
        NAME monitor_test
        COMMAND $<TARGET_FILE:monitor_test>
)
