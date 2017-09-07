add_library(networking
    impl/peer_communication_service_impl.cpp
    )

target_link_libraries(networking
    rxcpp
    model
    ordering_service
    synchronizer
    logger
    )

add_library(block_loader
    impl/block_loader_impl.cpp
    )
target_link_libraries(block_loader
    pb_model_converters
    loader_grpc
    rxcpp
    model
    )

add_library(block_loader_service
    impl/block_loader_service.cpp
    )
target_link_libraries(block_loader_service
    loader_grpc
    ametsuchi
    )
