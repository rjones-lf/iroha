set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

addtest(simulator_test simulator_test.cpp)
target_link_libraries(simulator_test
     simulator
        )

