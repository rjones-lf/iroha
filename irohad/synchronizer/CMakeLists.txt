add_library(synchronizer
    impl/synchronizer_impl.cpp
    )

target_link_libraries(synchronizer
    shared_model_proto_backend
    rxcpp
    logger
    )
