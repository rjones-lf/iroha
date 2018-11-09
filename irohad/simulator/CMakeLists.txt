add_library(simulator
    impl/simulator.cpp
    )

target_link_libraries(simulator
    consensus_round
    shared_model_interfaces
    rxcpp
    logger
    common
    )
