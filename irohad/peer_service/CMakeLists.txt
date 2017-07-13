#ADD_LIBRARY(izanami STATIC
#    izanami.cpp
#)

#target_link_libraries(izanami
#    hash
#    thread_pool
#    config_manager
#    peer_service
#    executor
#    connection_with_grpc
#    transaction_repository
#)

ADD_LIBRARY(peer_service STATIC
    self_status.cpp
    peer_service.cpp
)

target_link_libraries(peer_service
    timer
    time64
    logger
    crypto
    model
    schema
)