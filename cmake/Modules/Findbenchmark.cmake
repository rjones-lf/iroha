add_library(benchmark UNKNOWN IMPORTED)

find_path(benchmark_INCLUDE_DIR benchmark/benchmark.h)
mark_as_advanced(benchmark_INCLUDE_DIR)

find_library(benchmark_LIBRARY benchmark)
mark_as_advanced(benchmark_LIBRARY)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(benchmark DEFAULT_MSG
    benchmark_INCLUDE_DIR
    benchmark_LIBRARY
    )

iroha_get_lib_name(BENCHLIB benchmark STATIC)

if (NOT benchmark_FOUND)
  externalproject_add(google_benchmark
      GIT_REPOSITORY https://github.com/google/benchmark
      GIT_TAG v1.2.0
      CMAKE_ARGS
          -G${CMAKE_GENERATOR}
          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
          -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
          -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
      BUILD_BYPRODUCTS
          ${CMAKE_BINARY_DIR}${XCODE_EXT}/${BENCHLIB}
      INSTALL_COMMAND  "" # remove install step
      TEST_COMMAND     "" # remove test step
      UPDATE_COMMAND   "" # remove update step
      )
  externalproject_get_property(google_benchmark source_dir binary_dir)
  set(benchmark_INCLUDE_DIR ${source_dir}/include)

  set(benchmark_LIBRARY ${CMAKE_BINARY_DIR}${XCODE_EXT}/${BENCHLIB})

  file(MAKE_DIRECTORY ${benchmark_INCLUDE_DIR})

  add_dependencies(benchmark google_benchmark)
endif ()

set_target_properties(benchmark PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${benchmark_INCLUDE_DIR}
    IMPORTED_LOCATION ${benchmark_LIBRARY}
    INTERFACE_LINK_LIBRARIES "pthread"
    )
