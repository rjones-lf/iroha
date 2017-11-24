addtest(ametsuchi_test ametsuchi_test.cpp)
target_link_libraries(ametsuchi_test
    ametsuchi
    generator
    libs_common
    )

addtest(wsv_query_command_test wsv_query_command_test.cpp)
target_link_libraries(wsv_query_command_test
    ametsuchi
    libs_common
    )

addtest(flat_file_test flat_file_test.cpp)
target_link_libraries(flat_file_test
    ametsuchi
    libs_common
    )

addtest(block_query_test block_query_test.cpp)
target_link_libraries(block_query_test
        ametsuchi
        libs_common
        )

add_library(ametsuchi_fixture INTERFACE)
target_link_libraries(ametsuchi_fixture INTERFACE
    pqxx
    cpp_redis
    )
