
SET(AERON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")

include_directories(
  ${AERON_PATH}/aeron-client/src/main/cpp
)
link_directories(
    ${AERON_PATH}/cppbuild/Release/lib
)

ADD_LIBRARY(connection_with_aeron STATIC connection_with_aeron.cpp)
target_link_libraries(connection_with_aeron
  aeron_client
  logger
  peer_service_with_json
)

