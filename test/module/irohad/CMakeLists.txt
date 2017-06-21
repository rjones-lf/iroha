set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

# Reusable tests
add_subdirectory(ametsuchi)
add_subdirectory(logger)
add_subdirectory(peer_service)
