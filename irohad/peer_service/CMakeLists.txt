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
    monitor.cpp
    change_state.cpp
    self_state.cpp
    synchronizer/synchronizer.cpp
)

target_link_libraries(peer_service
    timer
    time64
    logger
    crypto
    schema
)