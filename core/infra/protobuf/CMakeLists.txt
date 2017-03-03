link_directories(
    /usr/local/lib
)

ADD_LIBRARY(event_with_grpc STATIC
    api.grpc.pb.cc
    api.pb.cc
)
target_link_libraries(event_with_grpc
    grpc++
    grpc++_reflection
    protobuf
    pthread
    dl
    ed25519
    hash
)
