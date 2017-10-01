add_executable(benchmark_result
    result.cpp

    )
target_link_libraries(benchmark_result
    ${Boost_LIBRARIES}
    benchmark
    )
