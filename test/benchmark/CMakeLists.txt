# directory contains benchmarks

# default benchmark path is build/benchmark_bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark_bin)


add_executable(benchmark_example
    benchmark_example.cpp
    )
target_link_libraries(benchmark_example
    benchmark
    shared_model_stateless_validation
    )

add_executable(bm_proto_creation
    bm_proto_creation.cpp
    )

target_include_directories(bm_proto_creation PUBLIC
    ${PROJECT_SOURCE_DIR}/test
    )

target_link_libraries(bm_proto_creation
    benchmark
    shared_model_proto_backend
    )

