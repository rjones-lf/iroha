SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

include_directories(
  ${JSON_PATH}/src
)

ADD_LIBRARY(json_parse_with_nlohman  STATIC
  json_parse_with_nlohman.cpp
)