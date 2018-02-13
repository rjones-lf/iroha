SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

add_subdirectory(module)
add_subdirectory(framework)
add_subdirectory(integration)
add_subdirectory(system)

if(BENCHMARKING)
  add_subdirectory(benchmark)
endif()
