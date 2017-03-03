link_directories(
    ${CRYPTO_PATH}
    /usr/local/lib
)

ADD_LIBRARY(connection_with_grpc STATIC
  connection_with_grpc.cpp
)

target_link_libraries(connection_with_grpc
  event_with_grpc
  json
)
