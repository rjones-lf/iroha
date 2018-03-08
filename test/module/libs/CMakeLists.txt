set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

# Reusable tests
add_subdirectory(amount)
add_subdirectory(cache)
add_subdirectory(crypto)
add_subdirectory(datetime)
add_subdirectory(validator)
add_subdirectory(converter)
add_subdirectory(common)
