# Temporary comment-outed for compilation
#compile_proto_to_cpp     (api.proto)
#compile_proto_to_grpc_cpp(api_service.proto)

compile_proto_to_cpp(block.proto)
compile_proto_to_cpp(primitive.proto)
compile_proto_to_cpp(commands.proto)
compile_proto_to_grpc_cpp(endpoint.proto)

add_dependencies(commandsproto_h primitiveproto_h)
add_dependencies(blockproto_h commandsproto_h)
add_dependencies(endpointproto_h blockproto_h)

add_library(schema
    block.pb.cc
    commands.pb.cc
    primitive.pb.cc
)
target_link_libraries(schema
    protobuf
)

add_library(endpoint
    endpoint.pb.cc
    endpoint.grpc.pb.cc
)
target_link_libraries(endpoint
    protobuf
    grpc
    grpc++
    grpc++_reflection
)
