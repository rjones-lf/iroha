SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)



###########################
#      Sumeragi test      #
###########################

add_executable(test_sumeragi
    test_sumeragi.cpp
)

target_link_libraries(test_sumeragi
    connection_with_grpc_flatbuffer
)