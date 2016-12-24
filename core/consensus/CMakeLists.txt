SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
)

include_directories(
  "${PROJECT_SOURCE_DIR}/core/vendor/thread_pool_cpp/thread_pool"
)

target_link_libraries(sumeragi
  peer_service_with_json
  connection_with_grpc
  signature
  event_repository
  commands
  objects
)

ADD_LIBRARY(consensus_event STATIC
  consensus_event.cpp
)

target_link_libraries(consensus_event
  commands
  objects
  transaction
  validator
  hash
)