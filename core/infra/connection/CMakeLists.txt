link_directories(
    ${CRYPTO_PATH}
    /usr/local/lib
)

ADD_LIBRARY(connection_with_grpc STATIC
  connection_with_grpc.cpp
)

target_link_libraries(connection_with_grpc
  signature
  event_with_grpc
  core_repository
  transaction_repository
  config_manager
  peer_service
)
