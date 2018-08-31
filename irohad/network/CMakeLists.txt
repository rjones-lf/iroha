add_library(networking
    impl/peer_communication_service_impl.cpp
    )

target_link_libraries(networking
    rxcpp
    shared_model_interfaces
    ordering_service
    synchronizer
    logger
    )

add_library(block_loader
    impl/block_loader_impl.cpp
    )

target_link_libraries(block_loader
    loader_grpc
    rxcpp
    shared_model_interfaces
    shared_model_proto_backend
    schema
    logger
    )

add_library(block_loader_service
    impl/block_loader_service.cpp
    )
target_link_libraries(block_loader_service
    loader_grpc
    ametsuchi
    )
