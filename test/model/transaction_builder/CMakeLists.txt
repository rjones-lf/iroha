set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

set(CMAKE_BUILD_TYPE Debug)
set(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

include_directories(
    ${GTEST_INCLUDE_DIR}
)
link_directories(
    ${PROJECT_BINARY_DIR}/lib
    ${LEVELDB_PATH}/out-static
)

add_executable(transaction_builder_test
    transaction_builder_test.cpp
)
target_link_libraries(transaction_builder_test
    transaction
    exception
    ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread
)

add_test(
    NAME transaction_builder_test
    COMMAND $<TARGET_FILE:transaction_builder_test>
)
