add_executable(benchmark_result
    result_optional.cpp

    )
target_link_libraries(benchmark_result
    ${Boost_LIBRARIES}
    optional
    benchmark
    )
