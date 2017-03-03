add_library(config_manager STATIC
  peer_service_with_json.cpp
  iroha_config_with_json.cpp
  config_format.cpp
)
target_link_libraries(config_manager
  json
  exception
  logger
)
