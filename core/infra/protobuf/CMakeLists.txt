SET(CMAKE_CXX_FLAGS "-std=c++14 -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl")

link_directories(
    /usr/local/lib
)


ADD_LIBRARY(event_with_grpc STATIC
    ../protobuf/event.grpc.pb.cc
    ../protobuf/event.pb.cc
)

target_link_libraries(event_with_grpc
    grpc++
    grpc++_reflection
    protobuf
    pthread
    dl
    hash
)

ADD_LIBRARY(convertor STATIC
    convertor.cpp
    ../protobuf/event.grpc.pb.cc
    ../protobuf/event.pb.cc
)