
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

#ADD_LIBRARY(peer_service STATIC
#    peer_service.cpp
#    hijiri.cpp
#)

#target_link_libraries(peer_service
#    exception
#    logger
#    config_manager
#    transaction_builder
#    connection_with_grpc
#    transaction_repository

    # I wonder why this linkage is needed. If it's removed, linker error occurs.
#    http_server_with_cappuccino
#)