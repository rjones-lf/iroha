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

ADD_LIBRARY(membership_service STATIC
    peer_service.cpp
    hijiri.cpp
)

target_link_libraries(membership_service
    exception
    logger
    config_manager
    flatbuffer_service
    connection_with_grpc_flatbuffer
)