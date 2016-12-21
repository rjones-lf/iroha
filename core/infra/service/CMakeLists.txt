
set(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

include_directories(
        ${JSON_PATH}/src
)

ADD_LIBRARY(peer_service_with_json STATIC
    peer_service_with_json.cpp
)

target_link_libraries(peer_service_with_json
    logger
)