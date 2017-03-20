set(PROTOBUF_SRC_DIR "${PROJECT_SOURCE_DIR}/core/infra/protobuf")

file(MAKE_DIRECTORY ${PROTOBUF_SRC_DIR})
execute_process(COMMAND bash "-c" "cd $IROHA_HOME/protos; protoc  --cpp_out=${PROTOBUF_SRC_DIR}/ *.proto")
execute_process(COMMAND bash "-c" "cd $IROHA_HOME/protos; protoc -I./  --grpc_out=${PROTOBUF_SRC_DIR}/ --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin *.proto")

add_library(event_with_grpc STATIC
    ${PROTOBUF_SRC_DIR}/api.grpc.pb.cc
    ${PROTOBUF_SRC_DIR}/api.pb.cc
)

target_link_libraries(event_with_grpc
    grpc++
    grpc++_reflection
    protobuf
    ed25519
    hash
)
