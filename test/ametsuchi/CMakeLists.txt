set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

#add_subdirectory(ametsuchi)

addtest(bstore_test block_store/block_store_flat_test.cpp)
target_link_libraries(bstore_test
    block_store
    )

addtest(block_index_test index/block_index_test.cpp)
target_link_libraries(block_index_test
    index
    )

addtest(tx_index_test index/tx_index_test.cpp)
target_link_libraries(tx_index_test
    index
    )

addtest(wsv_test wsv/wsv_test.cpp)
target_link_libraries(wsv_test
    wsv
    )

addtest(ametsuchi_test ametsuchi_test.cpp)
target_link_libraries(ametsuchi_test
    storage
    )