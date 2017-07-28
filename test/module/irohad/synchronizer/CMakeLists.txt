set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

addtest(synchronizer_test synchronizer_test.cpp)
target_link_libraries(synchronizer_test
      synchronizer
      )


