SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/fuzz_bin)
SET(CMAKE_CXX_FLAGS "${LIBFUZZER_FLAGS_BASE} -fsanitize=address,undefined -fsanitize-coverage=trace-pc-guard,trace-cmp,trace-gep,trace-div")

add_subdirectory(core/crypto)
