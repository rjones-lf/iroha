SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

###########################
#        grpc test        #
###########################
add_executable(grpc_test
    grpc_test.cpp
)
target_link_libraries(grpc_test
    endpoint_fbs
    gtest
)
add_test(
    NAME grpc_test
    COMMAND $<TARGET_FILE:grpc_test>
)