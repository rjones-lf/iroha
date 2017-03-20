ADD_LIBRARY(http_server_with_cappuccino STATIC 
  http_server_with_cappuccino.cpp
)

target_link_libraries(http_server_with_cappuccino
  json
  hash
  cappuccino
  asio
  pthread
  logger
  peer_service
  config_manager
  transaction_builder
  event_with_grpc
)
