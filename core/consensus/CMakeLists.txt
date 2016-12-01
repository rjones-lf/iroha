SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
)
target_link_libraries(sumeragi
  json_parse_with_nlohman
  peer_service_with_json
  connection_with_grpc
  signature
  event_repository
  objects
)

ADD_LIBRARY(consensus_event STATIC
  consensus_event.cpp
)

target_link_libraries(consensus_event
  commands
  objects
  transaction
  hash
)