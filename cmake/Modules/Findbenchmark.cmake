find_path(benchmark_INCLUDE_DIRS benchmark/benchmark.h)

find_library(benchmark_LIBRARIES benchmark)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(benchmark
  REQUIRED_VARS benchmark_INCLUDE_DIRS benchmark_LIBRARIES
  )

