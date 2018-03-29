addtest(consensus_sunny_day consensus_sunny_day.cpp)
target_link_libraries(consensus_sunny_day
    yac
    model
    shared_model_stateless_validation
    )
