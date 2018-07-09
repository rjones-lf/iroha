# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

set(SCHEMA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../shared_model/schema/)
compile_proto_only_grpc_to_cpp(endpoint.proto "-I${CMAKE_CURRENT_SOURCE_DIR}/../shared_model/schema/")
set(SCHEMA_PATH ${CMAKE_CURRENT_SOURCE_DIR})
compile_proto_to_grpc_cpp(yac.proto)
compile_proto_to_grpc_cpp(ordering.proto "-I${CMAKE_CURRENT_SOURCE_DIR}/../shared_model/schema/")
compile_proto_to_grpc_cpp(loader.proto "-I${CMAKE_CURRENT_SOURCE_DIR}/../shared_model/schema/")
compile_proto_to_grpc_cpp(mst.proto "-I${CMAKE_CURRENT_SOURCE_DIR}/../shared_model/schema/")

add_library(endpoint
    # ${SCHEMA_OUT_DIR}/endpoint.pb.cc
    ${SCHEMA_OUT_DIR}/endpoint.grpc.pb.cc
    )
target_link_libraries(endpoint
    protobuf
    grpc++
    schema
    )

add_library(yac_grpc
    yac.pb.cc
    yac.grpc.pb.cc
    )
target_link_libraries(yac_grpc
    protobuf
    grpc++
    )

add_library(ordering_grpc
    ordering.pb.cc
    ordering.grpc.pb.cc
    )
target_link_libraries(ordering_grpc
    schema
    grpc++
    )

add_library(loader_grpc
    loader.pb.cc
    loader.grpc.pb.cc
    )
target_link_libraries(loader_grpc
    schema
    grpc++
    )

add_library(mst_grpc
    mst.pb.cc
    mst.grpc.pb.cc
    )
target_link_libraries(mst_grpc
    schema
    grpc++
    )
