########################################################################################
# Executable executor_test
########################################################################################

# ToDo

########################################################################################
# Executable flatbuffer_service
########################################################################################
add_executable(flatbuffer_service_test flatbuffer_service_test.cpp)
target_link_libraries(flatbuffer_service_test
    flatbuffer_service
    gtest
    membership_service
)
add_test(
        NAME flatbuffer_service_test
        COMMAND $<TARGET_FILE:flatbuffer_service_test>
)