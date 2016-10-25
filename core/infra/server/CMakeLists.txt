
set(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

include_directories(
  ${JSON_PATH}/src
)

ADD_LIBRARY(http_server_with_cappuccino STATIC http_server_with_cappuccino.cpp)
target_link_libraries(http_server_with_cappuccino
  boost_system
  ssl
  crypto
)