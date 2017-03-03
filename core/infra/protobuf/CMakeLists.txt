SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(CMAKE_CXX_FLAGS "-g -std=c++1y -Wall -fPIC -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl")

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
