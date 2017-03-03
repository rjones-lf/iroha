include_directories(
  ${JSON_PATH}/src
)

ADD_LIBRARY(http_server_with_cappuccino STATIC 
  http_server_with_cappuccino.cpp
)

target_link_libraries(http_server_with_cappuccino
  ssl
  logger
  datetime
  base64
  signature
  connection_with_grpc
  cappuccino
)
