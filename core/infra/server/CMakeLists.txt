set(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

include_directories(
  ${JSON_PATH}/src
)

ADD_LIBRARY(http_server_with_cappuccino STATIC http_server_with_cappuccino.cpp)
target_link_libraries(http_server_with_cappuccino
  ssl
  crypto
  logger
  datetime
  peer_service_with_json

  objects
  base64
  signature
  convertor
  connection_with_grpc
  peer_service_with_json

  event_repository
)

