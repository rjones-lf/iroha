add_library(simulator
    impl/simulator.cpp
    )

target_link_libraries(simulator
    crypto_provider
    model
    rxcpp
    optional
    logger
    )
