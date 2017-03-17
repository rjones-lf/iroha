include_directories(
  ${PROJECT_SOURCE_DIR}/core
)

# hash benchmark
add_executable(hash_benchmark
  hash.cpp
)
target_link_libraries(hash_benchmark
  benchmark
  hash
)


# base64 benchmark
add_executable(base64_benchmark
  base64.cpp
)
target_link_libraries(base64_benchmark
  benchmark
)
