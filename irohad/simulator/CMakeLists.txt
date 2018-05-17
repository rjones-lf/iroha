add_library(simulator
    impl/simulator.cpp
    )

target_link_libraries(simulator
    shared_model_proto_backend
    rxcpp
    logger
    )
