
SET(CMAKE_CXX_FLAGS "-std=c++14 -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl")
SET(CRYPTO_PATH  "${PROJECT_SOURCE_DIR}/core/crypto")


link_directories(
    ${CRYPTO_PATH}
    /usr/local/lib
)


ADD_LIBRARY(connection_with_grpc STATIC
  connection_with_grpc.cpp
)

target_link_libraries(connection_with_grpc
  event_with_grpc
)
