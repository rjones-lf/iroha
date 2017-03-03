ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
)

target_link_libraries(sumeragi
  config_manager
  connection_with_grpc
  signature
  thread_pool
  executor
  merkle_transaction_repository
)
