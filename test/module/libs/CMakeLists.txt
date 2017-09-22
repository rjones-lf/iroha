set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

# Reusable tests
add_subdirectory(amount)
add_subdirectory(crypto)
add_subdirectory(datetime)
add_subdirectory(map_queue)


addtest(result_test result_test.cpp)
target_link_libraries(result_test ${Boost_LIBRARIES})
