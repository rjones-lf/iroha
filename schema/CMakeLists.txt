#
# Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
# http://soramitsu.co.jp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

compile_proto_to_cpp(block.proto)
compile_proto_to_cpp(proposal.proto)
compile_proto_to_cpp(primitive.proto)
compile_proto_to_cpp(commands.proto)
compile_proto_to_cpp(queries.proto)
compile_proto_to_cpp(responses.proto)
compile_proto_to_cpp(endpoint.proto)

if (IROHA_ROOT_PROJECT)
  compile_proto_to_grpc_cpp(endpoint.proto)
  compile_proto_to_grpc_cpp(yac.proto)
  compile_proto_to_grpc_cpp(ordering.proto)
  compile_proto_to_grpc_cpp(loader.proto)
endif ()

add_library(schema
    block.pb.cc
    commands.pb.cc
    primitive.pb.cc
    queries.pb.cc
    responses.pb.cc
    endpoint.pb.cc
    proposal.pb.cc
    )

target_link_libraries(schema
    protobuf
    )
target_include_directories(schema PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

if (IROHA_ROOT_PROJECT)
  add_library(endpoint
      endpoint.pb.cc
      endpoint.grpc.pb.cc
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
endif ()
