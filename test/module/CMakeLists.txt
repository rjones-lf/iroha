set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

# Reusable tests
add_subdirectory(crypto)
add_subdirectory(irohad/logger)
add_subdirectory(vendor)
add_subdirectory(peer_service)
