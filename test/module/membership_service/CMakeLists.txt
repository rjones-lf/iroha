#
# Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

########################################################################################
# PeerService
########################################################################################
add_executable(peer_service_test peer_service_test.cpp)
target_link_libraries(peer_service_test
  gtest
  membership_service

)
add_test(
  NAME peer_service_test
  COMMAND $<TARGET_FILE:peer_service_test>
)

add_executable(peer_service_to_issue_transaction_test peer_service_to_issue_transaction_test.cpp)
target_link_libraries(peer_service_to_issue_transaction_test
  gtest
  endpoint_fbs
  membership_service
  flatbuffer_service
  connection_with_grpc_flatbuffer
)
add_test(
  NAME peer_service_to_issue_transaction_test
  COMMAND $<TARGET_FILE:peer_service_to_issue_transaction_test>
)

add_executable(synchornizer_connection_part_test synchornizer_connection_part_test.cpp)
target_link_libraries(synchornizer_connection_part_test
  gtest
  repository
  endpoint_fbs
  membership_service
  flatbuffer_service
  connection_with_grpc_flatbuffer
)
add_test(
  NAME synchornizer_connection_part_test
  COMMAND $<TARGET_FILE:peer_service_to_issue_transaction_test>
)