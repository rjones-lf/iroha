add_library(config_utils STATIC
  config_utils.cpp
)

target_link_libraries(config_utils
  logger
)

add_library(config_manager STATIC
  peer_service_with_json.cpp
  iroha_config_with_json.cpp
  config_format.cpp
)

target_link_libraries(config_manager
  config_utils
  expected
  logger
  json
  ip_tools
)
